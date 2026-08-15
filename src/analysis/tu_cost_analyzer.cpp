#include <compileforge/analysis/tu_cost_analyzer.hpp>
#include <algorithm>
#include <sstream>

namespace compileforge {

std::string tu_cost_tier_to_string(TUCostTier tier) {
    switch (tier) {
        case TUCostTier::Low: return "LOW";
        case TUCostTier::Medium: return "MEDIUM";
        case TUCostTier::High: return "HIGH";
        case TUCostTier::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

TUCostProfile TUCostAnalyzer::analyze_tu(const FileNode& node, const DependencyGraph& graph) {
    TUCostProfile profile;
    profile.relative_path = node.relative_path;
    profile.source_loc = node.metrics.sloc;
    profile.direct_headers = node.fan_stats.fan_out_direct;
    profile.transitive_headers = node.fan_stats.fan_out_transitive;
    profile.macro_density = node.metrics.macro_count;
    profile.template_count = node.metrics.template_count;

    // Calculate deterministic cost score
    double score = static_cast<double>(profile.source_loc) * 0.2 +
                   static_cast<double>(profile.transitive_headers) * 5.0 +
                   static_cast<double>(profile.macro_density) * 2.0 +
                   static_cast<double>(profile.template_count) * 3.0;

    profile.cost_score = score;

    if (score < 50.0) {
        profile.tier = TUCostTier::Low;
    } else if (score < 150.0) {
        profile.tier = TUCostTier::Medium;
    } else if (score < 300.0) {
        profile.tier = TUCostTier::High;
    } else {
        profile.tier = TUCostTier::Critical;
    }

    std::ostringstream ss;
    ss << tu_cost_tier_to_string(profile.tier) << " (Score: " << static_cast<int>(score)
       << " | SLOC: " << profile.source_loc << ", Transitive Headers: " << profile.transitive_headers << ")";
    profile.explanation = ss.str();

    return profile;
}

std::vector<TUCostProfile> TUCostAnalyzer::analyze_all_tus(const DependencyGraph& graph) {
    std::vector<TUCostProfile> profiles;
    for (const auto& path : graph.all_nodes()) {
        const auto* node = graph.get_node(path);
        if (node && node->kind == FileKind::TranslationUnit) {
            profiles.push_back(analyze_tu(*node, graph));
        }
    }

    std::sort(profiles.begin(), profiles.end(), [](const TUCostProfile& a, const TUCostProfile& b) {
        return a.cost_score > b.cost_score;
    });

    return profiles;
}

} // namespace compileforge
