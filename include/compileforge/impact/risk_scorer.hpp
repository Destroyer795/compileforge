#ifndef COMPILEFORGE_IMPACT_RISK_SCORER_HPP
#define COMPILEFORGE_IMPACT_RISK_SCORER_HPP

#include <string>
#include <vector>
#include "impact_analyzer.hpp"
#include "../graph/dependency_graph.hpp"

namespace compileforge {

struct RiskScoreBreakdown {
    int total_risk_score{0}; // 0 to 100
    int impact_factor{0};
    int build_cost_factor{0};
    int architecture_factor{0};
    int git_churn_factor{0};
    int complexity_factor{0};
    int cycle_factor{0};
};

struct ReviewHotspot {
    std::string relative_path;
    std::string risk_level; // HIGH, MEDIUM, LOW
    std::string why_reason;
    double score{0.0};
};

struct ChangeRiskResult {
    RiskScoreBreakdown score_breakdown;
    std::vector<std::string> why_risky_reasons;
    std::vector<ReviewHotspot> review_hotspots;
};

class RiskScorer {
public:
    static ChangeRiskResult compute_risk(
        const ImpactAnalysisResult& impact,
        const DependencyGraph& graph,
        size_t cycle_count
    );
};

} // namespace compileforge

#endif // COMPILEFORGE_IMPACT_RISK_SCORER_HPP
