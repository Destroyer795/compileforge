#include "../../include/compileforge/analysis/build_config_health.hpp"
#include <unordered_map>
#include <unordered_set>
#include <sstream>

namespace compileforge {

std::vector<BuildConfigFinding> BuildConfigHealthAnalyzer::analyze(const CompilationDatabase& db) {
    std::vector<BuildConfigFinding> findings;
    if (db.entries().empty()) return findings;

    std::unordered_map<std::string, size_t> opt_levels;
    std::unordered_map<std::string, size_t> std_levels;
    size_t debug_count = 0;
    size_t no_warning_count = 0;

    for (const auto& entry : db.entries()) {
        auto inv = CompilerInvocationAnalyzer::parse_arguments(entry.arguments);
        opt_levels[inv.optimization_level]++;
        if (!inv.language_standard.empty()) {
            std_levels[inv.language_standard]++;
        }
        if (inv.has_debug_symbols) debug_count++;
        if (inv.warning_flags.empty()) no_warning_count++;
    }

    // Check mixed optimization levels
    if (opt_levels.size() > 1) {
        BuildConfigFinding f;
        f.severity = FindingSeverity::Warning;
        f.category = "Optimization Inconsistency";
        std::ostringstream ss;
        ss << db.entries().size() << " translation units use " << opt_levels.size() << " different optimization settings";
        f.message = ss.str();
        f.actionable_recommendation = "Harmonize -O flags across translation units in CMake target configurations.";
        findings.push_back(f);
    }

    // Check mixed language standards
    if (std_levels.size() > 1) {
        BuildConfigFinding f;
        f.severity = FindingSeverity::Warning;
        f.category = "Standard Inconsistency";
        std::ostringstream ss;
        ss << "Multiple C++ standards detected across project translation units";
        f.message = ss.str();
        f.actionable_recommendation = "Set CMAKE_CXX_STANDARD globally to enforce standard consistency.";
        findings.push_back(f);
    }

    // Check missing warnings
    if (no_warning_count > 0) {
        BuildConfigFinding f;
        f.severity = FindingSeverity::Warning;
        f.category = "Warning Flags Missing";
        std::ostringstream ss;
        ss << no_warning_count << " translation units lack recommended compiler warning flags (-Wall -Wextra)";
        f.message = ss.str();
        f.actionable_recommendation = "Enable -Wall -Wextra or /W4 globally across all targets.";
        findings.push_back(f);
    }

    return findings;
}

} // namespace compileforge
