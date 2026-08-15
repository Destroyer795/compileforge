#include <compileforge/metrics/source_metrics.hpp>

namespace compileforge {

ProjectSummary SourceMetrics::compute_summary(const DependencyGraph& graph, size_t circular_count) {
    ProjectSummary summary;
    summary.circular_dependency_count = circular_count;

    for (const auto& path : graph.all_nodes()) {
        const auto* node = graph.get_node(path);
        if (!node) continue;

        summary.total_files++;
        if (node->kind == FileKind::Header) {
            summary.total_headers++;
            if (node->fan_stats.fan_out_transitive > 15) {
                summary.high_fanout_header_count++;
            }
        } else if (node->kind == FileKind::TranslationUnit) {
            summary.total_translation_units++;
        }

        summary.total_loc += node->metrics.total_lines;
        summary.total_sloc += node->metrics.sloc;
        summary.total_estimated_build_seconds += node->build_time.compilation_seconds;

        if (node->git_data.commit_count > 5 && node->metrics.cyclomatic_complexity > 10) {
            summary.high_churn_complexity_count++;
        }
    }

    return summary;
}

} // namespace compileforge
