#include <compileforge/recommendations/recommendation_engine.hpp>
#include <compileforge/core/utils.hpp>
#include <algorithm>
#include <sstream>

namespace compileforge {

std::vector<Recommendation> RecommendationEngine::generate_recommendations(
    const DependencyGraph& graph,
    const std::vector<DependencyCycle>& cycles,
    const Config& config
) {
    std::vector<Recommendation> recs;
    size_t priority_idx = 1;

    // 1. Circular Dependencies (Highest priority)
    if (!cycles.empty()) {
        for (const auto& cycle : cycles) {
            Recommendation r;
            std::ostringstream ss_id;
            ss_id << "PRIORITY " << priority_idx++;
            r.id = ss_id.str();
            r.severity = Severity::High;
            r.target_file = cycle.cycle_path.empty() ? "" : cycle.cycle_path.front();
            r.title = "Remove circular include dependency loop";

            std::ostringstream ss_desc;
            ss_desc << "Cycle detected (" << cycle.length << " files): ";
            for (size_t i = 0; i < cycle.cycle_path.size(); ++i) {
                ss_desc << cycle.cycle_path[i];
                if (i + 1 < cycle.cycle_path.size()) ss_desc << " -> ";
            }
            r.description = ss_desc.str();
            r.actionable_step = "Use forward declarations or split shared definitions into a separate light interface header.";
            recs.push_back(r);
        }
    }

    // 2. High Fan-In Headers
    for (const auto& path : graph.all_nodes()) {
        const auto* node = graph.get_node(path);
        if (!node) continue;

        if (node->kind == FileKind::Header && node->fan_stats.fan_in_transitive >= config.fan_in_threshold) {
            Recommendation r;
            std::ostringstream ss_id;
            ss_id << "PRIORITY " << priority_idx++;
            r.id = ss_id.str();
            r.severity = Severity::High;
            r.target_file = node->relative_path;
            r.title = "Investigate heavyweight common header: " + node->relative_path;

            std::ostringstream ss_desc;
            ss_desc << "Header has " << node->fan_stats.fan_in_transitive << " transitive dependents. Modifications trigger widespread rebuilds.";
            r.description = ss_desc.str();
            r.actionable_step = "PImpl pattern or split header into focused micro-headers to minimize blast radius.";
            recs.push_back(r);
        }
    }

    // Sort recommendations by severity and priority
    std::sort(recs.begin(), recs.end(), [](const Recommendation& a, const Recommendation& b) {
        if (a.severity != b.severity) {
            return static_cast<int>(a.severity) > static_cast<int>(b.severity);
        }
        return a.id < b.id;
    });

    return recs;
}

} // namespace compileforge
