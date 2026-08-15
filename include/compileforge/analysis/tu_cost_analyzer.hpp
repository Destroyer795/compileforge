#ifndef COMPILEFORGE_ANALYSIS_TU_COST_ANALYZER_HPP
#define COMPILEFORGE_ANALYSIS_TU_COST_ANALYZER_HPP

#include <string>
#include <vector>
#include "../core/types.hpp"
#include "../graph/dependency_graph.hpp"

namespace compileforge {

enum class TUCostTier {
    Low,
    Medium,
    High,
    Critical
};

std::string tu_cost_tier_to_string(TUCostTier tier);

struct TUCostProfile {
    std::string relative_path;
    size_t source_loc{0};
    size_t direct_headers{0};
    size_t transitive_headers{0};
    size_t macro_density{0};
    size_t template_count{0};
    double cost_score{0.0};
    TUCostTier tier{TUCostTier::Low};
    std::string explanation;
};

class TUCostAnalyzer {
public:
    static TUCostProfile analyze_tu(const FileNode& node, const DependencyGraph& graph);
    static std::vector<TUCostProfile> analyze_all_tus(const DependencyGraph& graph);
};

} // namespace compileforge

#endif // COMPILEFORGE_ANALYSIS_TU_COST_ANALYZER_HPP
