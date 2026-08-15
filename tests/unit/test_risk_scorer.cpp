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

TEST_CASE(test_risk_scorer_determinism) {
    ImpactAnalysisResult impact;
    impact.total_affected_tus = 5;
    impact.percentage_tus_affected = 25.0;
    impact.max_impact_depth = 2;

    DependencyGraph graph;
    auto res1 = RiskScorer::compute_risk(impact, graph, 1);
    auto res2 = RiskScorer::compute_risk(impact, graph, 1);

    ASSERT_EQ(res1.score_breakdown.total_risk_score, res2.score_breakdown.total_risk_score);
    ASSERT_EQ(res1.why_risky_reasons.size(), res2.why_risky_reasons.size());
}

TEST_CASE(test_risk_scorer_monotonicity) {
    DependencyGraph graph;

    ImpactAnalysisResult small_impact;
    small_impact.total_affected_tus = 1;
    small_impact.percentage_tus_affected = 5.0;
    small_impact.max_impact_depth = 1;

    ImpactAnalysisResult large_impact;
    large_impact.total_affected_tus = 50;
    large_impact.percentage_tus_affected = 80.0;
    large_impact.max_impact_depth = 6;

    auto r_small = RiskScorer::compute_risk(small_impact, graph, 0);
    auto r_large = RiskScorer::compute_risk(large_impact, graph, 2);

    ASSERT_TRUE(r_large.score_breakdown.total_risk_score > r_small.score_breakdown.total_risk_score);
}
