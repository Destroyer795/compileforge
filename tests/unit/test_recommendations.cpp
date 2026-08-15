#include <tests/test_framework.hpp>
#include <compileforge/recommendations/recommendation_engine.hpp>

using namespace compileforge;

TEST_CASE(test_recommendation_engine) {
    DependencyGraph graph;

    FileNode header;
    header.relative_path = "include/common.hpp";
    header.kind = FileKind::Header;
    header.fan_stats.fan_in_transitive = 50;

    graph.add_node(header);

    std::vector<DependencyCycle> cycles;
    auto recs = RecommendationEngine::generate_recommendations(graph, cycles, Config::default_config());

    ASSERT_TRUE(!recs.empty());
    ASSERT_EQ(recs[0].target_file, "include/common.hpp");
}
