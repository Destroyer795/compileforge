#include <compileforge/impact/impact_analyzer.hpp>
#include <compileforge/core/utils.hpp>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace compileforge {

ImpactAnalysisResult ImpactAnalyzer::analyze_impact(
    const DependencyGraph& graph,
    const std::vector<ChangedFileEntry>& changed_files
) {
    ImpactAnalysisResult result;
    result.changed_files = changed_files;
    if (changed_files.empty() || graph.node_count() == 0) {
        result.cost_estimate.has_measured_timings = false;
        result.cost_estimate.basis_explanation = "No changed files or empty project graph.";
        return result;
    }

    std::unordered_map<std::string, size_t> visited_distances;
    std::queue<std::pair<std::string, size_t>> queue;

    for (const auto& chg : changed_files) {
        std::string norm = utils::normalize_path(chg.relative_path);
        // Find matching graph node (direct or relative match)
        for (const auto& path : graph.all_nodes()) {
            if (utils::path_equals(path, chg.relative_path) || utils::ends_with(path, chg.relative_path)) {
                queue.push({path, 0});
                visited_distances[path] = 0;
            }
        }
    }

    size_t total_proj_tus = 0;
    size_t total_proj_sloc = 0;
    for (const auto& path : graph.all_nodes()) {
        const auto* n = graph.get_node(path);
        if (n) {
            if (n->kind == FileKind::TranslationUnit) total_proj_tus++;
            total_proj_sloc += n->metrics.sloc;
        }
    }

    size_t affected_loc = 0;

    // BFS forward traversal along dependents graph
    while (!queue.empty()) {
        auto [curr_node_path, dist] = queue.front();
        queue.pop();

        result.max_impact_depth = std::max(result.max_impact_depth, dist);
        const auto* node = graph.get_node(curr_node_path);

        if (node) {
            AffectedNode aff;
            aff.relative_path = node->relative_path;
            aff.kind = node->kind;
            aff.distance = dist;
            aff.is_translation_unit = (node->kind == FileKind::TranslationUnit);
            aff.is_hotspot = (node->hotspot.total_score >= 50.0);
            aff.sloc = node->metrics.sloc;
            result.affected_nodes.push_back(aff);

            if (node->kind == FileKind::TranslationUnit) {
                result.total_affected_tus++;
            } else if (node->kind == FileKind::Header) {
                result.total_affected_headers++;
            }
            affected_loc += node->metrics.sloc;

            // Visit dependents
            auto dependents = graph.get_transitive_dependents(curr_node_path);
            for (const auto& dep : dependents) {
                if (visited_distances.find(dep) == visited_distances.end()) {
                    visited_distances[dep] = dist + 1;
                    queue.push({dep, dist + 1});
                }
            }
        }
    }

    result.total_affected_files = result.affected_nodes.size();
    if (total_proj_tus > 0) {
        result.percentage_tus_affected = (static_cast<double>(result.total_affected_tus) / static_cast<double>(total_proj_tus)) * 100.0;
    }
    if (total_proj_sloc > 0) {
        result.percentage_loc_affected = (static_cast<double>(affected_loc) / static_cast<double>(total_proj_sloc)) * 100.0;
    }

    if (result.percentage_tus_affected >= 50.0) {
        result.impact_classification = "CRITICAL";
    } else if (result.percentage_tus_affected >= 20.0) {
        result.impact_classification = "HIGH";
    } else if (result.percentage_tus_affected >= 5.0) {
        result.impact_classification = "MEDIUM";
    } else {
        result.impact_classification = "LOW";
    }

    // Build Cost Estimation
    result.cost_estimate.has_measured_timings = false;
    result.cost_estimate.basis_explanation = "No measured build trace timings available for affected files.";

    return result;
}

} // namespace compileforge
