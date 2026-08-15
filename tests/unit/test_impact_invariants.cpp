#include <tests/test_framework.hpp>
#include <compileforge/impact/impact_analyzer.hpp>
#include <compileforge/graph/dependency_graph.hpp>
#include <queue>
#include <unordered_set>

using namespace compileforge;

static bool has_path(const DependencyGraph& graph, const std::string& src, const std::string& dst) {
    if (src == dst) return true;
    std::queue<std::string> q;
    std::unordered_set<std::string> visited;
    q.push(src);
    visited.insert(src);

    while (!q.empty()) {
        std::string curr = q.front();
        q.pop();
        if (curr == dst) return true;

        for (const auto& next : graph.get_incoming_edges(curr)) {
            if (visited.find(next) == visited.end()) {
                visited.insert(next);
                q.push(next);
            }
        }
    }
    return false;
}

TEST_CASE(test_impact_graph_topological_invariants) {
    DependencyGraph graph;

    FileNode h1; h1.relative_path = "include/a.hpp"; h1.kind = FileKind::Header;
    FileNode h2; h2.relative_path = "include/b.hpp"; h2.kind = FileKind::Header;
    FileNode tu1; tu1.relative_path = "src/tu1.cpp"; tu1.kind = FileKind::TranslationUnit;
    FileNode tu2; tu2.relative_path = "src/unrelated.cpp"; tu2.kind = FileKind::TranslationUnit;

    graph.add_node(h1);
    graph.add_node(h2);
    graph.add_node(tu1);
    graph.add_node(tu2);

    graph.add_edge("src/tu1.cpp", "include/b.hpp");
    graph.add_edge("include/b.hpp", "include/a.hpp");
    graph.compute_fan_stats();

    std::vector<ChangedFileEntry> changed;
    ChangedFileEntry c; c.relative_path = "include/a.hpp"; c.change_kind = FileChangeKind::Modified;
    changed.push_back(c);

    auto impact = ImpactAnalyzer::analyze_impact(graph, changed);

    // Invariant verification: Every reported affected node must have a reachable path from a.hpp
    for (const auto& aff : impact.affected_nodes) {
        ASSERT_TRUE(has_path(graph, "include/a.hpp", aff.relative_path));
    }

    // Unrelated TU must not be in affected nodes
    for (const auto& aff : impact.affected_nodes) {
        ASSERT_TRUE(aff.relative_path != "src/unrelated.cpp");
    }
}
