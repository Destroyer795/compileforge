#ifndef COMPILEFORGE_METRICS_SOURCE_METRICS_HPP
#define COMPILEFORGE_METRICS_SOURCE_METRICS_HPP

#include <compileforge/core/types.hpp>
#include <compileforge/graph/dependency_graph.hpp>

namespace compileforge {

class SourceMetrics {
public:
    static ProjectSummary compute_summary(const DependencyGraph& graph, size_t circular_count);
};

} // namespace compileforge

#endif // COMPILEFORGE_METRICS_SOURCE_METRICS_HPP
