#include <tests/test_framework.hpp>
#include <compileforge/graph/dependency_graph.hpp>

using namespace compileforge;

TEST_CASE(test_dependency_graph_fan_stats) {
    DependencyGraph graph;

    FileNode tu;
    tu.relative_path = "src/main.cpp";
    tu.kind = FileKind::TranslationUnit;
    tu.metrics.total_lines = 100;
    graph.add_node(tu);

    FileNode h1;
    h1.relative_path = "include/a.hpp";
    h1.kind = FileKind::Header;
    h1.metrics.total_lines = 200;
    graph.add_node(h1);

    FileNode h2;
    h2.relative_path = "include/b.hpp";
    h2.kind = FileKind::Header;
    h2.metrics.total_lines = 50;
    graph.add_node(h2);

    graph.add_edge("src/main.cpp", "include/a.hpp");
    graph.add_edge("include/a.hpp", "include/b.hpp");

    graph.compute_fan_stats();

    const auto* node_tu = graph.get_node("src/main.cpp");
    ASSERT_TRUE(node_tu != nullptr);
    ASSERT_EQ(node_tu->fan_stats.fan_out_direct, 1);
    ASSERT_EQ(node_tu->fan_stats.fan_out_transitive, 2);

    const auto* node_b = graph.get_node("include/b.hpp");
    ASSERT_TRUE(node_b != nullptr);
    ASSERT_EQ(node_b->fan_stats.fan_in_direct, 1);
    ASSERT_EQ(node_b->fan_stats.fan_in_transitive, 2); // a.hpp and main.cpp
}
