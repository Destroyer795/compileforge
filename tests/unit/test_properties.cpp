#include <tests/test_framework.hpp>
#include <compileforge/graph/dependency_graph.hpp>
#include <compileforge/graph/cycle_detector.hpp>
#include <compileforge/analysis/hotspot_scorer.hpp>

using namespace compileforge;

TEST_CASE(test_property_graph_duality_invariant) {
    DependencyGraph graph;

    FileNode n1; n1.relative_path = "src/main.cpp"; n1.canonical_path = "/src/main.cpp"; n1.kind = FileKind::TranslationUnit;
    FileNode n2; n2.relative_path = "include/a.hpp"; n2.canonical_path = "/include/a.hpp"; n2.kind = FileKind::Header;
    FileNode n3; n3.relative_path = "include/b.hpp"; n3.canonical_path = "/include/b.hpp"; n3.kind = FileKind::Header;

    graph.add_node(n1);
    graph.add_node(n2);
    graph.add_node(n3);

    graph.add_edge("src/main.cpp", "include/a.hpp");
    graph.add_edge("include/a.hpp", "include/b.hpp");

    auto a_deps = graph.get_transitive_dependencies("src/main.cpp");
    ASSERT_TRUE(a_deps.find("include/a.hpp") != a_deps.end());
    ASSERT_TRUE(a_deps.find("include/b.hpp") != a_deps.end());

    auto b_dependents = graph.get_transitive_dependents("include/b.hpp");
    ASSERT_TRUE(b_dependents.find("include/a.hpp") != b_dependents.end());
    ASSERT_TRUE(b_dependents.find("src/main.cpp") != b_dependents.end());
}

TEST_CASE(test_property_hotspot_scoring_determinism) {
    DependencyGraph graph;

    FileNode n1;
    n1.relative_path = "include/common.hpp";
    n1.kind = FileKind::Header;
    n1.metrics.total_lines = 1000;
    n1.fan_stats.fan_in_transitive = 40;
    n1.git_data.commit_count = 15;

    FileNode n2;
    n2.relative_path = "src/render.cpp";
    n2.kind = FileKind::TranslationUnit;
    n2.metrics.total_lines = 300;
    n2.fan_stats.fan_in_transitive = 0;
    n2.git_data.commit_count = 5;

    graph.add_node(n1);
    graph.add_node(n2);

    HotspotScorer::compute_hotspots(graph);
    double score1_run1 = graph.get_node("include/common.hpp")->hotspot.total_score;
    double score2_run1 = graph.get_node("src/render.cpp")->hotspot.total_score;

    // Run again
    HotspotScorer::compute_hotspots(graph);
    double score1_run2 = graph.get_node("include/common.hpp")->hotspot.total_score;
    double score2_run2 = graph.get_node("src/render.cpp")->hotspot.total_score;

    ASSERT_DOUBLE_EQ(score1_run1, score1_run2);
    ASSERT_DOUBLE_EQ(score2_run1, score2_run2);
}

TEST_CASE(test_property_scc_cycle_invariant) {
    DependencyGraph graph;

    FileNode a; a.relative_path = "include/a.hpp"; a.canonical_path = "/path/a.hpp"; a.kind = FileKind::Header;
    FileNode b; b.relative_path = "include/b.hpp"; b.canonical_path = "/path/b.hpp"; b.kind = FileKind::Header;
    FileNode c; c.relative_path = "include/c.hpp"; c.canonical_path = "/path/c.hpp"; c.kind = FileKind::Header;

    graph.add_node(a);
    graph.add_node(b);
    graph.add_node(c);

    graph.add_edge("include/a.hpp", "include/b.hpp");
    graph.add_edge("include/b.hpp", "include/c.hpp");
    graph.add_edge("include/c.hpp", "include/a.hpp");

    auto cycles = CycleDetector::detect_cycles(graph);
    ASSERT_EQ(cycles.size(), 1);
    ASSERT_EQ(cycles[0].length, 3);
    ASSERT_EQ(cycles[0].cycle_path.size(), 4);
    ASSERT_EQ(cycles[0].cycle_path[0], cycles[0].cycle_path[3]);
}
