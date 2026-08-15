#include "../test_framework.hpp"
#include "../../include/compileforge/impact/impact_analyzer.hpp"
#include "../../include/compileforge/graph/dependency_graph.hpp"

using namespace compileforge;

TEST_CASE(test_impact_leaf_source_change) {
    DependencyGraph graph;

    FileNode tu1; tu1.relative_path = "src/client.cpp"; tu1.kind = FileKind::TranslationUnit; tu1.metrics.sloc = 100;
    FileNode tu2; tu2.relative_path = "src/server.cpp"; tu2.kind = FileKind::TranslationUnit; tu2.metrics.sloc = 200;
    FileNode h1;  h1.relative_path = "include/types.hpp"; h1.kind = FileKind::Header; h1.metrics.sloc = 50;

    graph.add_node(tu1);
    graph.add_node(tu2);
    graph.add_node(h1);

    graph.add_edge("src/client.cpp", "include/types.hpp");
    graph.add_edge("src/server.cpp", "include/types.hpp");
    graph.compute_fan_stats();

    std::vector<ChangedFileEntry> changed;
    ChangedFileEntry c; c.relative_path = "src/client.cpp"; c.change_kind = FileChangeKind::Modified;
    changed.push_back(c);

    auto res = ImpactAnalyzer::analyze_impact(graph, changed);
    ASSERT_EQ(res.total_affected_tus, 1);
    ASSERT_EQ(res.total_affected_headers, 0);
    ASSERT_DOUBLE_EQ(res.percentage_tus_affected, 50.0);
}

TEST_CASE(test_impact_shared_header_change) {
    DependencyGraph graph;

    FileNode tu1; tu1.relative_path = "src/client.cpp"; tu1.kind = FileKind::TranslationUnit; tu1.metrics.sloc = 100;
    FileNode tu2; tu2.relative_path = "src/server.cpp"; tu2.kind = FileKind::TranslationUnit; tu2.metrics.sloc = 200;
    FileNode h1;  h1.relative_path = "include/types.hpp"; h1.kind = FileKind::Header; h1.metrics.sloc = 50;

    graph.add_node(tu1);
    graph.add_node(tu2);
    graph.add_node(h1);

    graph.add_edge("src/client.cpp", "include/types.hpp");
    graph.add_edge("src/server.cpp", "include/types.hpp");
    graph.compute_fan_stats();

    std::vector<ChangedFileEntry> changed;
    ChangedFileEntry c; c.relative_path = "include/types.hpp"; c.change_kind = FileChangeKind::Modified;
    changed.push_back(c);

    auto res = ImpactAnalyzer::analyze_impact(graph, changed);
    ASSERT_EQ(res.total_affected_tus, 2);
    ASSERT_EQ(res.total_affected_headers, 1);
    ASSERT_DOUBLE_EQ(res.percentage_tus_affected, 100.0);
    ASSERT_EQ(res.max_impact_depth, 1);
}

TEST_CASE(test_impact_multiple_changed_files) {
    DependencyGraph graph;

    FileNode tu1; tu1.relative_path = "src/a.cpp"; tu1.kind = FileKind::TranslationUnit; tu1.metrics.sloc = 100;
    FileNode tu2; tu2.relative_path = "src/b.cpp"; tu2.kind = FileKind::TranslationUnit; tu2.metrics.sloc = 100;
    FileNode h1;  h1.relative_path = "include/a.hpp"; h1.kind = FileKind::Header; h1.metrics.sloc = 20;
    FileNode h2;  h2.relative_path = "include/b.hpp"; h2.kind = FileKind::Header; h2.metrics.sloc = 20;

    graph.add_node(tu1);
    graph.add_node(tu2);
    graph.add_node(h1);
    graph.add_node(h2);

    graph.add_edge("src/a.cpp", "include/a.hpp");
    graph.add_edge("src/b.cpp", "include/b.hpp");
    graph.compute_fan_stats();

    std::vector<ChangedFileEntry> changed;
    ChangedFileEntry c1; c1.relative_path = "include/a.hpp"; c1.change_kind = FileChangeKind::Modified;
    ChangedFileEntry c2; c2.relative_path = "include/b.hpp"; c2.change_kind = FileChangeKind::Modified;
    changed.push_back(c1);
    changed.push_back(c2);

    auto res = ImpactAnalyzer::analyze_impact(graph, changed);
    ASSERT_EQ(res.total_affected_tus, 2);
    ASSERT_EQ(res.total_affected_headers, 2);
}

TEST_CASE(test_impact_deleted_or_untracked_file) {
    DependencyGraph graph;
    FileNode tu1; tu1.relative_path = "src/main.cpp"; tu1.kind = FileKind::TranslationUnit; tu1.metrics.sloc = 100;
    graph.add_node(tu1);
    graph.compute_fan_stats();

    std::vector<ChangedFileEntry> changed;
    ChangedFileEntry c; c.relative_path = "include/deleted.hpp"; c.change_kind = FileChangeKind::Deleted;
    changed.push_back(c);

    auto res = ImpactAnalyzer::analyze_impact(graph, changed);
    ASSERT_EQ(res.total_affected_tus, 0);
    ASSERT_EQ(res.total_affected_files, 0);
}

TEST_CASE(test_impact_circular_dependency_termination) {
    DependencyGraph graph;
    FileNode h1; h1.relative_path = "include/c1.hpp"; h1.kind = FileKind::Header;
    FileNode h2; h2.relative_path = "include/c2.hpp"; h2.kind = FileKind::Header;
    FileNode tu1; tu1.relative_path = "src/main.cpp"; tu1.kind = FileKind::TranslationUnit;

    graph.add_node(h1);
    graph.add_node(h2);
    graph.add_node(tu1);

    graph.add_edge("include/c1.hpp", "include/c2.hpp");
    graph.add_edge("include/c2.hpp", "include/c1.hpp");
    graph.add_edge("src/main.cpp", "include/c1.hpp");
    graph.compute_fan_stats();

    std::vector<ChangedFileEntry> changed;
    ChangedFileEntry c; c.relative_path = "include/c1.hpp"; c.change_kind = FileChangeKind::Modified;
    changed.push_back(c);

    auto res = ImpactAnalyzer::analyze_impact(graph, changed);
    ASSERT_EQ(res.total_affected_tus, 1);
    ASSERT_EQ(res.total_affected_headers, 2);
}

TEST_CASE(test_impact_cost_estimation_honesty) {
    DependencyGraph graph;
    FileNode tu1;
    tu1.relative_path = "src/main.cpp";
    tu1.kind = FileKind::TranslationUnit;
    tu1.build_time.is_measured = true;
    tu1.build_time.compilation_seconds = 2.45;

    graph.add_node(tu1);
    graph.compute_fan_stats();

    std::vector<ChangedFileEntry> changed;
    ChangedFileEntry c; c.relative_path = "src/main.cpp"; c.change_kind = FileChangeKind::Modified;
    changed.push_back(c);

    auto res = ImpactAnalyzer::analyze_impact(graph, changed);
    ASSERT_TRUE(res.cost_estimate.has_measured_timings);
    ASSERT_DOUBLE_EQ(res.cost_estimate.estimated_compile_seconds, 2.45);
    ASSERT_EQ(res.cost_estimate.confidence, "HIGH");
}
