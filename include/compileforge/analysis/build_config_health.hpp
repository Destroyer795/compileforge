#ifndef COMPILEFORGE_ANALYSIS_BUILD_CONFIG_HEALTH_HPP
#define COMPILEFORGE_ANALYSIS_BUILD_CONFIG_HEALTH_HPP

#include <string>
#include <vector>
#include <compileforge/parser/compilation_database.hpp>
#include <compileforge/parser/compiler_invocation.hpp>

namespace compileforge {

enum class FindingSeverity {
    Warning,
    Info
};

struct BuildConfigFinding {
    FindingSeverity severity{FindingSeverity::Warning};
    std::string category;
    std::string message;
    std::string actionable_recommendation;
};

class BuildConfigHealthAnalyzer {
public:
    static std::vector<BuildConfigFinding> analyze(const CompilationDatabase& db);
};

} // namespace compileforge

#endif // COMPILEFORGE_ANALYSIS_BUILD_CONFIG_HEALTH_HPP
