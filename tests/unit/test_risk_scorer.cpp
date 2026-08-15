#include <tests/test_framework.hpp>
#include <compileforge/impact/risk_scorer.hpp>

using namespace compileforge;

TEST_CASE(test_risk_scorer_scoring) {
    ImpactAnalysisResult impact;
    impact.total_affected_tus = 10;
    impact.percentage_tus_affected = 50.0;
    impact.max_impact_depth = 5;

    DependencyGraph graph;
    auto res = RiskScorer::compute_risk(impact, graph, 0);

    ASSERT_TRUE(res.score_breakdown.total_risk_score >= 50);
    ASSERT_TRUE(!res.why_risky_reasons.empty());
}
