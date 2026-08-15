#include "../test_framework.hpp"
#include "../../include/compileforge/analysis/build_health_score.hpp"

using namespace compileforge;

TEST_CASE(test_build_health_score_computation) {
    DependencyGraph graph;
    std::vector<BuildConfigFinding> findings;

    // Clean project score
    auto res1 = BuildHealthScorer::compute_score(graph, 0, findings);
    ASSERT_EQ(res1.score, 100);

    // Project with 1 cycle (-15 penalty)
    auto res2 = BuildHealthScorer::compute_score(graph, 1, findings);
    ASSERT_EQ(res2.score, 85);
}
