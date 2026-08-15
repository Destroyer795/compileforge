#include <compileforge/impact/risk_scorer.hpp>
#include <compileforge/core/utils.hpp>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace compileforge {

ChangeRiskResult RiskScorer::compute_risk(
    const ImpactAnalysisResult& impact,
    const DependencyGraph& graph,
    size_t cycle_count
) {
    ChangeRiskResult result;
    RiskScoreBreakdown& bd = result.score_breakdown;

    // 1. Impact Factor (up to +35 pts based on % TUs affected)
    bd.impact_factor = static_cast<int>(std::min(35.0, impact.percentage_tus_affected * 0.7));
    if (impact.total_affected_tus > 0) {
        std::ostringstream ss;
        ss << "affects " << impact.total_affected_tus << " translation unit(s) ("
           << std::fixed << std::setprecision(1) << impact.percentage_tus_affected << "% of project)";
        result.why_risky_reasons.push_back(ss.str());
    }

    // 2. Build Cost & Depth Factor (up to +20 pts)
    bd.build_cost_factor = static_cast<int>(std::min(20.0, static_cast<double>(impact.max_impact_depth) * 3.0));
    if (impact.max_impact_depth > 3) {
        std::ostringstream ss;
        ss << "transitive impact reaches " << impact.max_impact_depth << " dependency levels deep";
        result.why_risky_reasons.push_back(ss.str());
    }

    // 3. Architecture & Centrality Factor (up to +20 pts)
    size_t hotspot_affected_count = 0;
    for (const auto& aff : impact.affected_nodes) {
        if (aff.is_hotspot) hotspot_affected_count++;
    }
    bd.architecture_factor = static_cast<int>(std::min(20.0, static_cast<double>(hotspot_affected_count) * 5.0));
    if (hotspot_affected_count > 0) {
        std::ostringstream ss;
        ss << "affects " << hotspot_affected_count << " high-centrality architectural hotspot file(s)";
        result.why_risky_reasons.push_back(ss.str());
    }

    // 4. Git Churn & Complexity Factor (up to +15 pts)
    size_t max_churn = 0;
    size_t max_complexity = 0;
    for (const auto& chg : impact.changed_files) {
        const auto* node = graph.get_node(chg.relative_path);
        if (node) {
            max_churn = std::max(max_churn, node->git_data.commit_count);
            max_complexity = std::max(max_complexity, node->metrics.cyclomatic_complexity);
        }
    }
    bd.git_churn_factor = static_cast<int>(std::min(10.0, static_cast<double>(max_churn) * 1.0));
    bd.complexity_factor = static_cast<int>(std::min(5.0, static_cast<double>(max_complexity) * 0.5));

    if (max_churn > 10) {
        std::ostringstream ss;
        ss << "modifies a high-churn file (" << max_churn << " historical commits)";
        result.why_risky_reasons.push_back(ss.str());
    }

    // 5. Cycle Involvement Factor (up to +10 pts)
    if (cycle_count > 0) {
        bd.cycle_factor = static_cast<int>(std::min(10.0, static_cast<double>(cycle_count) * 10.0));
        std::ostringstream ss;
        ss << "touches files involved in " << cycle_count << " circular dependency loop(s)";
        result.why_risky_reasons.push_back(ss.str());
    }

    bd.total_risk_score = std::max(0, std::min(100, bd.impact_factor + bd.build_cost_factor + bd.architecture_factor + bd.git_churn_factor + bd.complexity_factor + bd.cycle_factor));

    // Compute Review Hotspots
    for (const auto& aff : impact.affected_nodes) {
        const auto* node = graph.get_node(aff.relative_path);
        if (!node) continue;

        double r_score = node->hotspot.total_score + static_cast<double>(aff.sloc) * 0.05 + static_cast<double>(node->fan_stats.fan_in_transitive) * 1.5;
        if (r_score >= 15.0) {
            ReviewHotspot rh;
            rh.relative_path = aff.relative_path;
            rh.score = r_score;

            if (r_score >= 50.0) {
                rh.risk_level = "HIGH";
                rh.why_reason = "High dependency fan-in + architectural centrality";
            } else if (r_score >= 25.0) {
                rh.risk_level = "MEDIUM";
                rh.why_reason = "Affected transitively + substantial LOC";
            } else {
                rh.risk_level = "LOW";
                rh.why_reason = "Modified file or direct dependent";
            }
            result.review_hotspots.push_back(rh);
        }
    }

    std::sort(result.review_hotspots.begin(), result.review_hotspots.end(), [](const ReviewHotspot& a, const ReviewHotspot& b) {
        if (a.score != b.score) return a.score > b.score;
        return a.relative_path < b.relative_path;
    });

    if (result.review_hotspots.size() > 7) {
        result.review_hotspots.resize(7);
    }

    return result;
}

} // namespace compileforge
