#ifndef COMPILEFORGE_GIT_GIT_ANALYZER_HPP
#define COMPILEFORGE_GIT_GIT_ANALYZER_HPP

#include <string>
#include <unordered_map>
#include <compileforge/core/result.hpp>
#include <compileforge/core/types.hpp>
#include <compileforge/graph/dependency_graph.hpp>

namespace compileforge {

class GitAnalyzer {
public:
    static bool is_git_repository(const std::string& repository_root);
    static Result<std::unordered_map<std::string, GitChurnData>> analyze_repository(const std::string& repository_root);
    static void enrich_graph(DependencyGraph& graph, const std::string& repository_root);
};

} // namespace compileforge

#endif // COMPILEFORGE_GIT_GIT_ANALYZER_HPP
