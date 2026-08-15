#ifndef COMPILEFORGE_REPORTING_REPORT_HPP
#define COMPILEFORGE_REPORTING_REPORT_HPP

#include <vector>
#include <string>
#include "../core/types.hpp"
#include "../graph/cycle_detector.hpp"
#include "../parser/compiler_invocation.hpp"
#include "../analysis/build_config_health.hpp"
#include "../analysis/include_optimizer.hpp"
#include "../analysis/tu_cost_analyzer.hpp"
#include "../analysis/build_health_score.hpp"
#include "../core/json.hpp"

namespace compileforge {

struct AnalysisReport {
    ProjectSummary summary;
    BuildHealthResult health_score;
    std::vector<FileNode> files;
    std::vector<FileNode> top_hotspots;
    std::vector<FileNode> top_fanin_headers;
    std::vector<DependencyCycle> cycles;
    std::vector<Recommendation> recommendations;
    std::vector<CompilerInvocationInfo> compiler_invocations;
    std::vector<BuildConfigFinding> build_config_findings;
    std::vector<IncludeIssue> include_issues;
    std::vector<TUCostProfile> tu_cost_profiles;
};

class TerminalReporter {
public:
    static void print(const AnalysisReport& report, bool use_colors = true);
};

class JsonReporter {
public:
    static JsonValue to_json(const AnalysisReport& report);
    static std::string render(const AnalysisReport& report, int indent = 2);
};

class HtmlReporter {
public:
    static std::string render(const AnalysisReport& report, const std::string& project_name = "CompileForge");
};

} // namespace compileforge

#endif // COMPILEFORGE_REPORTING_REPORT_HPP
