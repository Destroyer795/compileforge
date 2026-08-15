#ifndef COMPILEFORGE_REPORTING_IMPACT_HTML_REPORTER_HPP
#define COMPILEFORGE_REPORTING_IMPACT_HTML_REPORTER_HPP

#include <string>
#include "../impact/impact_analyzer.hpp"
#include "../impact/risk_scorer.hpp"
#include "../core/json.hpp"

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
