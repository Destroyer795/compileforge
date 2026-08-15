#ifndef COMPILEFORGE_REPORTING_VALIDATION_REPORTER_HPP
#define COMPILEFORGE_REPORTING_VALIDATION_REPORTER_HPP

#include <string>
#include "../validation/impact_validator.hpp"
#include "../core/json.hpp"

namespace compileforge {

class ValidationReporter {
public:
    static void print_terminal(const ImpactValidationResult& result);
    static JsonValue to_json(const ImpactValidationResult& result);
    static std::string render_json(const ImpactValidationResult& result, int indent = 2);
    static std::string render_html(const ImpactValidationResult& result, const std::string& project_name = "CompileForge Project");
};

} // namespace compileforge

#endif // COMPILEFORGE_REPORTING_VALIDATION_REPORTER_HPP
