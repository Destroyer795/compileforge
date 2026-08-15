#include <tests/test_framework.hpp>
#include <compileforge/metrics/source_metrics.hpp>
#include <compileforge/build/build_trace.hpp>

using namespace compileforge;

TEST_CASE(test_source_metrics_computation) {
    DependencyGraph graph;

    FileNode tu;
    tu.relative_path = "src/main.cpp";
    tu.kind = FileKind::TranslationUnit;
    tu.metrics.total_lines = 150;
    tu.metrics.sloc = 110;
    tu.build_time.compilation_seconds = 0.5;
    graph.add_node(tu);

    FileNode header;
    header.relative_path = "include/app.hpp";
    header.kind = FileKind::Header;
    header.metrics.total_lines = 80;
    header.metrics.sloc = 60;
    header.fan_stats.fan_out_transitive = 20;
    graph.add_node(header);

    ProjectSummary summary = SourceMetrics::compute_summary(graph, 1);
    ASSERT_EQ(summary.total_files, 2);
    ASSERT_EQ(summary.total_headers, 1);
    ASSERT_EQ(summary.total_translation_units, 1);
    ASSERT_EQ(summary.total_loc, 230);
    ASSERT_EQ(summary.total_sloc, 170);
    ASSERT_EQ(summary.circular_dependency_count, 1);
    ASSERT_EQ(summary.high_fanout_header_count, 1);
}

TEST_CASE(test_build_trace_time_estimation) {
    DependencyGraph graph;

    FileNode tu;
    tu.relative_path = "src/render.cpp";
    tu.kind = FileKind::TranslationUnit;
    tu.metrics.total_lines = 500;
    tu.metrics.template_count = 10;
    tu.metrics.cyclomatic_complexity = 8;
    graph.add_node(tu);

    BuildTraceAnalyzer::estimate_build_times(graph);

    const auto* node = graph.get_node("src/render.cpp");
    ASSERT_TRUE(node != nullptr);
    ASSERT_TRUE(node->build_time.compilation_seconds > 0.0);
}
