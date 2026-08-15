#include "../../include/compileforge/analysis/hotspot_scorer.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace compileforge {

void HotspotScorer::compute_hotspots(DependencyGraph& graph) {
    size_t max_fan_in = 1;
    size_t max_loc = 1;
    size_t max_complexity = 1;
    size_t max_churn = 1;

    for (const auto& path : graph.all_nodes()) {
        const auto* node = graph.get_node(path);
        if (!node) continue;

        max_fan_in = std::max(max_fan_in, node->fan_stats.fan_in_transitive);
        max_loc = std::max(max_loc, node->metrics.total_lines);
        max_complexity = std::max(max_complexity, node->metrics.cyclomatic_complexity);
        max_churn = std::max(max_churn, node->git_data.commit_count);
    }

    for (const auto& path : graph.all_nodes()) {
        auto* node = graph.get_node_mut(path);
        if (!node) continue;

        double fan_in_f = (static_cast<double>(node->fan_stats.fan_in_transitive) / static_cast<double>(max_fan_in)) * 30.0;
        double build_f = (static_cast<double>(node->metrics.total_lines) / static_cast<double>(max_loc)) * 35.0;
        double complexity_f = (static_cast<double>(node->metrics.cyclomatic_complexity) / static_cast<double>(max_complexity)) * 20.0;
        double churn_f = (static_cast<double>(node->git_data.commit_count) / static_cast<double>(max_churn)) * 15.0;

        node->hotspot.build_time_factor = build_f;
        node->hotspot.fan_in_factor = fan_in_f;
        node->hotspot.complexity_factor = complexity_f;
        node->hotspot.churn_factor = churn_f;
        node->hotspot.total_score = std::min(100.0, build_f + fan_in_f + complexity_f + churn_f);

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << "BUILD: " << build_f << "/35, DEPENDENCY: " << fan_in_f << "/30, COMPLEXITY: "
           << complexity_f << "/20, CHURN: " << churn_f << "/15";
        node->hotspot.score_breakdown = ss.str();
    }
}

std::vector<FileNode> HotspotScorer::get_top_hotspots(const DependencyGraph& graph, size_t limit) {
    std::vector<FileNode> nodes;
    for (const auto& path : graph.all_nodes()) {
        const auto* n = graph.get_node(path);
        if (n) nodes.push_back(*n);
    }

    std::sort(nodes.begin(), nodes.end(), [](const FileNode& a, const FileNode& b) {
        if (a.hotspot.total_score != b.hotspot.total_score) {
            return a.hotspot.total_score > b.hotspot.total_score;
        }
        return a.relative_path < b.relative_path; // Deterministic tie-breaking
    });

    if (nodes.size() > limit) {
        nodes.resize(limit);
    }
    return nodes;
}

} // namespace compileforge
