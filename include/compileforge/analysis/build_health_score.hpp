#ifndef COMPILEFORGE_ANALYSIS_BUILD_HEALTH_SCORE_HPP
#define COMPILEFORGE_ANALYSIS_BUILD_HEALTH_SCORE_HPP

#include <string>
#include <vector>
#include <compileforge/graph/dependency_graph.hpp>
#include <compileforge/analysis/build_config_health.hpp>

namespace compileforge {

struct BuildHealthResult {
    int score{100}; // 0 to 100
    std::vector<std::string> positive_factors;
    std::vector<std::string> negative_factors;
};

class BuildHealthScorer {
public:
    static BuildHealthResult compute_score(
        const DependencyGraph& graph,
        size_t cycle_count,
        const std::vector<BuildConfigFinding>& config_findings
    );
};

} // namespace compileforge

#endif // COMPILEFORGE_ANALYSIS_BUILD_HEALTH_SCORE_HPP
