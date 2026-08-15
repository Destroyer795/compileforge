#include "../test_framework.hpp"
#include "../../include/compileforge/graph/cycle_detector.hpp"

using namespace compileforge;

TEST_CASE(test_cycle_detector_simple_cycle) {
    DependencyGraph graph;

    FileNode a;
    a.relative_path = "include/a.hpp";
    a.canonical_path = "/path/a.hpp";
    a.kind = FileKind::Header;

    FileNode b;
    b.relative_path = "include/b.hpp";
    b.canonical_path = "/path/b.hpp";
    b.kind = FileKind::Header;

    FileNode c;
    c.relative_path = "include/c.hpp";
    c.canonical_path = "/path/c.hpp";
    c.kind = FileKind::Header;

    graph.add_node(a);
    graph.add_node(b);
    graph.add_node(c);

    // A -> B -> C -> A
    graph.add_edge("include/a.hpp", "include/b.hpp");
    graph.add_edge("include/b.hpp", "include/c.hpp");
    graph.add_edge("include/c.hpp", "include/a.hpp");

    auto cycles = CycleDetector::detect_cycles(graph);
    ASSERT_EQ(cycles.size(), 1);
    ASSERT_EQ(cycles[0].cycle_path.size(), 4);
    ASSERT_EQ(cycles[0].cycle_path[0], cycles[0].cycle_path[3]);
}
