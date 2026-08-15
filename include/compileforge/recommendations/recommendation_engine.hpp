#ifndef COMPILEFORGE_RECOMMENDATIONS_RECOMMENDATION_ENGINE_HPP
#define COMPILEFORGE_RECOMMENDATIONS_RECOMMENDATION_ENGINE_HPP

#include <vector>
#include <string>
#include "../core/types.hpp"
#include "../graph/dependency_graph.hpp"
#include "../graph/cycle_detector.hpp"
#include "../config/config.hpp"

namespace compileforge {

class RecommendationEngine {
public:
    static std::vector<Recommendation> generate_recommendations(
        const DependencyGraph& graph,
        const std::vector<DependencyCycle>& cycles,
        const Config& config
    );
};

} // namespace compileforge

#endif // COMPILEFORGE_RECOMMENDATIONS_RECOMMENDATION_ENGINE_HPP
