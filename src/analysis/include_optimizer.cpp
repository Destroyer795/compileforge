#include <compileforge/analysis/include_optimizer.hpp>
#include <sstream>

namespace compileforge {

std::vector<IncludeIssue> IncludeOptimizer::analyze_includes(const DependencyGraph& graph, size_t heavyweight_loc_threshold) {
    std::vector<IncludeIssue> issues;

    for (const auto& path : graph.all_nodes()) {
        const auto* node = graph.get_node(path);
        if (!node) continue;

        // Check self-includes and duplicate includes
        for (const auto& inc : node->includes) {
            if (inc.is_self_include) {
                IncludeIssue issue;
                issue.kind = IncludeIssueKind::SelfInclude;
                issue.source_file = node->relative_path;
                issue.target_file = inc.raw_path;
                issue.explanation = "File includes itself recursively";
                issues.push_back(issue);
            }
            if (inc.is_duplicate) {
                IncludeIssue issue;
                issue.kind = IncludeIssueKind::DuplicateInclude;
                issue.source_file = node->relative_path;
                issue.target_file = inc.raw_path;
                issue.explanation = "Header is included multiple times in the same file";
                issues.push_back(issue);
            }
        }

        // Heavyweight header check (transitive LOC > threshold & high fan-in)
        if (node->kind == FileKind::Header) {
            if (node->metrics.total_lines >= heavyweight_loc_threshold && node->fan_stats.fan_in_transitive >= 5) {
                IncludeIssue issue;
                issue.kind = IncludeIssueKind::HeavyweightHeader;
                issue.source_file = node->relative_path;
                issue.target_file = node->relative_path;
                issue.depth_or_loc = node->metrics.total_lines;
                std::ostringstream ss;
                ss << "Heavyweight header (" << node->metrics.total_lines << " LOC) included transitively by "
                   << node->fan_stats.fan_in_transitive << " files";
                issue.explanation = ss.str();
                issues.push_back(issue);
            }
        }

        // Deep transitive chain check
        if (node->fan_stats.fan_out_transitive > 15) {
            IncludeIssue issue;
            issue.kind = IncludeIssueKind::DeepTransitiveChain;
            issue.source_file = node->relative_path;
            issue.target_file = node->relative_path;
            issue.depth_or_loc = node->fan_stats.fan_out_transitive;
            std::ostringstream ss;
            ss << "Deep include hierarchy: transitively includes " << node->fan_stats.fan_out_transitive << " headers";
            issue.explanation = ss.str();
            issues.push_back(issue);
        }
    }

    return issues;
}

} // namespace compileforge
