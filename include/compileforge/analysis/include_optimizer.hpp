#ifndef COMPILEFORGE_ANALYSIS_INCLUDE_OPTIMIZER_HPP
#define COMPILEFORGE_ANALYSIS_INCLUDE_OPTIMIZER_HPP

#include <string>
#include <vector>
#include "../graph/dependency_graph.hpp"

namespace compileforge {

enum class IncludeIssueKind {
    DuplicateInclude,
    SelfInclude,
    HeavyweightHeader,
    HighFanOutHeader,
    DeepTransitiveChain,
    PotentiallyUnnecessaryIncludeCandidate // Clearly documented heuristic
};

struct IncludeIssue {
    IncludeIssueKind kind;
    std::string source_file;
    std::string target_file;
    std::string explanation;
    size_t depth_or_loc{0};
};

class IncludeOptimizer {
public:
    static std::vector<IncludeIssue> analyze_includes(const DependencyGraph& graph, size_t heavyweight_loc_threshold = 500);
};

} // namespace compileforge

#endif // COMPILEFORGE_ANALYSIS_INCLUDE_OPTIMIZER_HPP
