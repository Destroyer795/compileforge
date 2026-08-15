#include <tests/test_framework.hpp>
#include <compileforge/recommendations/recommendation_engine.hpp>

using namespace compileforge;

TEST_CASE(test_recommendation_engine) {
    DependencyGraph graph;

    FileNode header;
    header.relative_path = "include/big.hpp";
    header.kind = FileKind::Header;
    header.metrics.total_lines = 500;
    header.fan_stats.fan_in_transitive = 25;
    header.metrics.has_pragma_once = false;
    header.metrics.has_header_guard = false;
    graph.add_node(header);

    std::vector<DependencyCycle> cycles;
    DependencyCycle cycle;
    cycle.cycle_path = {"include/a.hpp", "include/b.hpp", "include/a.hpp"};
    cycle.length = 2;
    cycles.push_back(cycle);

    auto recs = RecommendationEngine::generate_recommendations(graph, cycles);
    ASSERT_TRUE(recs.size() >= 2);

    bool found_cycle_rec = false;
    bool found_header_rec = false;
    bool found_guard_rec = false;

    for (const auto& r : recs) {
        if (r.severity == Severity::High && r.title.find("circular") != std::string::npos) {
            found_cycle_rec = true;
        }
        if (r.target_file == "include/big.hpp" && r.severity == Severity::High) {
            found_header_rec = true;
        }
        if (r.target_file == "include/big.hpp" && r.severity == Severity::Medium) {
            found_guard_rec = true;
        }
    }

    ASSERT_TRUE(found_cycle_rec);
    ASSERT_TRUE(found_header_rec);
    ASSERT_TRUE(found_guard_rec);
}
