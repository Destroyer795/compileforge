#ifndef COMPILEFORGE_GRAPH_CYCLE_DETECTOR_HPP
#define COMPILEFORGE_GRAPH_CYCLE_DETECTOR_HPP

#include <vector>
#include "dependency_graph.hpp"
#include "../core/types.hpp"

namespace compileforge {

class CycleDetector {
public:
    static std::vector<DependencyCycle> detect_cycles(const DependencyGraph& graph);
};

} // namespace compileforge

#endif // COMPILEFORGE_GRAPH_CYCLE_DETECTOR_HPP
