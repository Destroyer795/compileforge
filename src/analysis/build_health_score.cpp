#include <compileforge/analysis/build_health_score.hpp>
#include <algorithm>
#include <sstream>

namespace compileforge {

BuildHealthResult BuildHealthScorer::compute_score(
    const DependencyGraph& graph,
    size_t cycle_count,
    const std::vector<BuildConfigFinding>& config_findings
) {
    BuildHealthResult res;
    int score = 100;

    // Deduct for cycles (-15 per cycle)
    if (cycle_count > 0) {
        int cycle_penalty = static_cast<int>(cycle_count * 15);
        score -= cycle_penalty;
        std::ostringstream ss;
        ss << "-" << cycle_penalty << " penalty: " << cycle_count << " circular dependency loop(s)";
        res.negative_factors.push_back(ss.str());
    } else {
        res.positive_factors.push_back("+0 circular dependencies (clean dependency DAG)");
    }

    // Deduct for config findings (-10 per finding)
    if (!config_findings.empty()) {
        int config_penalty = static_cast<int>(config_findings.size() * 10);
        score -= config_penalty;
        std::ostringstream ss;
        ss << "-" << config_penalty << " penalty: " << config_findings.size() << " build configuration inconsistency finding(s)";
        res.negative_factors.push_back(ss.str());
    } else {
        res.positive_factors.push_back("+consistent compiler flags across translation units");
    }

    // Check high fan-in headers (>20 dependents)
    size_t high_fanin_count = 0;
    for (const auto& path : graph.all_nodes()) {
        const auto* node = graph.get_node(path);
        if (node && node->kind == FileKind::Header && node->fan_stats.fan_in_transitive > 20) {
            high_fanin_count++;
        }
    }

    if (high_fanin_count > 0) {
        int fanin_penalty = static_cast<int>(high_fanin_count * 5);
        score -= fanin_penalty;
        std::ostringstream ss;
        ss << "-" << fanin_penalty << " penalty: " << high_fanin_count << " high fan-in header(s) (>20 dependents)";
        res.negative_factors.push_back(ss.str());
    } else {
        res.positive_factors.push_back("+good header dependency fan-in balance");
    }

    res.score = std::max(0, std::min(100, score));
    return res;
}

} // namespace compileforge
