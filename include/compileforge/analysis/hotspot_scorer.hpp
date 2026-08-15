#ifndef COMPILEFORGE_ANALYSIS_HOTSPOT_SCORER_HPP
#define COMPILEFORGE_ANALYSIS_HOTSPOT_SCORER_HPP

#include <vector>
#include "../graph/dependency_graph.hpp"
#include "../core/types.hpp"

namespace compileforge {

class HotspotScorer {
public:
    static void compute_hotspots(DependencyGraph& graph);
    static std::vector<FileNode> get_top_hotspots(const DependencyGraph& graph, size_t limit = 10);
};

} // namespace compileforge

#endif // COMPILEFORGE_ANALYSIS_HOTSPOT_SCORER_HPP
