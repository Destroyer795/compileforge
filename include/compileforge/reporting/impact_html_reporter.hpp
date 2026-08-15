#ifndef COMPILEFORGE_REPORTING_IMPACT_HTML_REPORTER_HPP
#define COMPILEFORGE_REPORTING_IMPACT_HTML_REPORTER_HPP

#include <string>
#include <compileforge/impact/impact_analyzer.hpp>
#include <compileforge/impact/risk_scorer.hpp>
#include <compileforge/core/json.hpp>

namespace compileforge {

struct ImpactReport {
    ImpactAnalysisResult impact;
    ChangeRiskResult risk;
};

class ImpactHtmlReporter {
public:
    static std::string render(const ImpactReport& report, const std::string& project_name = "CompileForge Impact");
};

class ImpactJsonReporter {
public:
    static JsonValue to_json(const ImpactReport& report);
    static std::string render(const ImpactReport& report, int indent = 2);
};

} // namespace compileforge

#endif // COMPILEFORGE_REPORTING_IMPACT_HTML_REPORTER_HPP
