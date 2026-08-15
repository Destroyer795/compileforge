#include <compileforge/reporting/report.hpp>
#include <compileforge/core/utils.hpp>
#include <iostream>
#include <iomanip>
#include <cstdlib>

namespace compileforge {

void TerminalReporter::print(const AnalysisReport& report, bool use_colors) {
    if (std::getenv("NO_COLOR") != nullptr || !utils::is_atty()) {
        use_colors = false;
    }

    const char* RESET   = use_colors ? "\033[0m" : "";
    const char* BOLD    = use_colors ? "\033[1m" : "";
    const char* RED     = use_colors ? "\033[31m" : "";
    const char* GREEN   = use_colors ? "\033[32m" : "";
    const char* YELLOW  = use_colors ? "\033[33m" : "";
    const char* CYAN    = use_colors ? "\033[36m" : "";
    const char* MAGENTA = use_colors ? "\033[35m" : "";

    std::cout << "\n" << BOLD << CYAN << "==========================================================" << RESET << "\n";
    std::cout << BOLD << CYAN << "               COMPILEFORGE BUILD INTELLIGENCE            " << RESET << "\n";
    std::cout << BOLD << CYAN << "==========================================================" << RESET << "\n\n";

    std::cout << BOLD << "BUILD HEALTH SCORE: " << (report.health_score.score >= 80 ? GREEN : (report.health_score.score >= 50 ? YELLOW : RED))
              << report.health_score.score << "/100" << RESET << "\n";
    for (const auto& pos : report.health_score.positive_factors) {
        std::cout << "  " << GREEN << pos << RESET << "\n";
    }
    for (const auto& neg : report.health_score.negative_factors) {
        std::cout << "  " << RED << neg << RESET << "\n";
    }
    std::cout << "\n";

    std::cout << BOLD << "PROJECT SUMMARY" << RESET << "\n";
    std::cout << "  Files:            " << report.summary.total_files << "\n";
    std::cout << "  Headers:          " << report.summary.total_headers << "\n";
    std::cout << "  Translation Units:" << report.summary.total_translation_units << "\n";
    std::cout << "  Lines of Code:    " << report.summary.total_loc << " (SLOC: " << report.summary.total_sloc << ")\n\n";

    if (!report.build_config_findings.empty()) {
        std::cout << BOLD << "BUILD CONFIGURATION HEALTH" << RESET << "\n";
        for (const auto& finding : report.build_config_findings) {
            std::cout << "  " << YELLOW << "WARNING: " << finding.category << RESET << " - " << finding.message << "\n";
            std::cout << "        -> " << finding.actionable_recommendation << "\n";
        }
        std::cout << "\n";
    }

    if (!report.tu_cost_profiles.empty()) {
        std::cout << BOLD << "TOP COSTLY TRANSLATION UNITS" << RESET << "\n";
        for (size_t i = 0; i < std::min<size_t>(5, report.tu_cost_profiles.size()); ++i) {
            const auto& prof = report.tu_cost_profiles[i];
            const char* tier_color = (prof.tier == TUCostTier::Critical ? RED : (prof.tier == TUCostTier::High ? YELLOW : CYAN));
            std::cout << "  " << std::left << std::setw(32) << prof.relative_path
                      << " " << tier_color << std::setw(8) << tu_cost_tier_to_string(prof.tier) << RESET
                      << " Transitive Headers: " << prof.transitive_headers << "\n";
        }
        std::cout << "\n";
    }

    std::cout << BOLD << "BUILD HOTSPOTS" << RESET << "\n";
    if (report.top_hotspots.empty()) {
        std::cout << "  No significant hotspots detected.\n";
    } else {
        for (size_t i = 0; i < std::min<size_t>(5, report.top_hotspots.size()); ++i) {
            const auto& node = report.top_hotspots[i];
            std::cout << "  " << std::left << std::setw(32) << node.relative_path
                      << " " << std::right << std::setw(6) << std::fixed << std::setprecision(2)
                      << node.build_time.compilation_seconds << "s"
                      << "  Score: " << std::setprecision(1) << node.hotspot.total_score << "\n";
        }
    }
    std::cout << "\n";

    std::cout << BOLD << "PRIORITIZED ACTION PLAN" << RESET << "\n";
    if (report.recommendations.empty()) {
        std::cout << "  " << GREEN << "No critical recommendations. Clean build architecture!" << RESET << "\n";
    } else {
        for (const auto& rec : report.recommendations) {
            const char* sev_color = (rec.severity == Severity::High) ? RED : (rec.severity == Severity::Medium ? YELLOW : CYAN);
            std::cout << "  " << sev_color << std::left << std::setw(12) << rec.id << RESET
                      << " " << rec.title << "\n";
            std::cout << "        Reason: " << rec.description << "\n";
            std::cout << "        Action: " << rec.actionable_step << "\n";
        }
    }
    std::cout << "\n";
}

} // namespace compileforge
