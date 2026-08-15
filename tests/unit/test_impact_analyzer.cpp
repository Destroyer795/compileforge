#include <tests/test_framework.hpp>
#include <compileforge/impact/impact_analyzer.hpp>
#include <compileforge/graph/dependency_graph.hpp>

using namespace compileforge;

TEST_CASE(test_impact_analyzer_transitive_traversal) {
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
    ASSERT_DOUBLE_EQ(res.percentage_tus_affected, 100.0);
}
