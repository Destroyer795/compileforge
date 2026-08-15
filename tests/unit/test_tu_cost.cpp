#include "../test_framework.hpp"
#include "../../include/compileforge/analysis/tu_cost_analyzer.hpp"

using namespace compileforge;

TEST_CASE(test_tu_cost_analyzer) {
    DependencyGraph graph;

    FileNode tu;
    tu.relative_path = "src/costly.cpp";
    tu.kind = FileKind::TranslationUnit;
    tu.metrics.sloc = 1000;
    tu.fan_stats.fan_out_transitive = 50;
    tu.metrics.macro_count = 20;

    graph.add_node(tu);

    auto prof = TUCostAnalyzer::analyze_tu(tu, graph);
    ASSERT_EQ(tu_cost_tier_to_string(prof.tier), "CRITICAL");
    ASSERT_TRUE(prof.cost_score > 300.0);
}
