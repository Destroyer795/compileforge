#ifndef COMPILEFORGE_REPORTING_REPORT_HPP
#define COMPILEFORGE_REPORTING_REPORT_HPP

#include <string>
#include <vector>
#include <compileforge/core/types.hpp>
#include <compileforge/graph/dependency_graph.hpp>
#include <compileforge/core/json.hpp>

namespace compileforge {

struct AnalysisReport {
    ProjectSummary summary;
    std::vector<FileNode> files;
    std::vector<DependencyCycle> cycles;
    std::vector<FileNode> top_hotspots;
    std::vector<FileNode> top_fanin_headers;
    std::vector<FileNode> top_churn_files;
    std::vector<Recommendation> recommendations;
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
    static std::string render(const AnalysisReport& report, const std::string& project_name = "CompileForge Analysis");
};

} // namespace compileforge

#endif // COMPILEFORGE_REPORTING_REPORT_HPP
