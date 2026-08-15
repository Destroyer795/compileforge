#include <tests/test_framework.hpp>
#include <compileforge/analysis/hotspot_scorer.hpp>

using namespace compileforge;

TEST_CASE(test_hotspot_scorer_ranking) {
    DependencyGraph graph;

    FileNode n1;
    n1.relative_path = "include/common.hpp";
    n1.kind = FileKind::Header;
    n1.metrics.total_lines = 1000;
    n1.fan_stats.fan_in_transitive = 50;
    n1.git_data.commit_count = 20;

    FileNode n2;
    n2.relative_path = "src/utils.cpp";
    n2.kind = FileKind::TranslationUnit;
    n2.metrics.total_lines = 50;
    n2.fan_stats.fan_in_transitive = 0;
    n2.git_data.commit_count = 1;

    graph.add_node(n1);
    graph.add_node(n2);

    HotspotScorer::compute_hotspots(graph);

    const auto* scored_n1 = graph.get_node("include/common.hpp");
    const auto* scored_n2 = graph.get_node("src/utils.cpp");

    ASSERT_TRUE(scored_n1 != nullptr);
    ASSERT_TRUE(scored_n2 != nullptr);
    ASSERT_TRUE(scored_n1->hotspot.total_score > scored_n2->hotspot.total_score);

    auto top = HotspotScorer::get_top_hotspots(graph, 1);
    ASSERT_EQ(top.size(), 1);
    ASSERT_EQ(top[0].relative_path, "include/common.hpp");
}
