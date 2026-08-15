#ifndef COMPILEFORGE_BUILD_BUILD_TRACE_HPP
#define COMPILEFORGE_BUILD_BUILD_TRACE_HPP

#include <string>
#include <unordered_map>
#include <compileforge/core/result.hpp>
#include <compileforge/core/types.hpp>
#include <compileforge/graph/dependency_graph.hpp>

namespace compileforge {

class BuildTraceAnalyzer {
public:
    static Result<std::unordered_map<std::string, double>> load_ftime_trace_file(const std::string& filepath);

    static void estimate_build_times(DependencyGraph& graph);
};

} // namespace compileforge

#endif // COMPILEFORGE_BUILD_BUILD_TRACE_HPP
