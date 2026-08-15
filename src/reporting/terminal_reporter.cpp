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

    std::cout << BOLD << "PROJECT SUMMARY" << RESET << "\n";
    std::cout << "  Files:            " << report.summary.total_files << "\n";
    std::cout << "  Headers:          " << report.summary.total_headers << "\n";
    std::cout << "  Translation Units:" << report.summary.total_translation_units << "\n";
    std::cout << "  Lines of Code:    " << report.summary.total_loc << " (SLOC: " << report.summary.total_sloc << ")\n\n";

    std::cout << BOLD << "BUILD HEALTH & HOTSPOTS" << RESET << "\n";
    if (report.top_hotspots.empty()) {
        std::cout << "  No significant hotspots detected.\n";
    } else {
        for (size_t i = 0; i < std::min<size_t>(5, report.top_hotspots.size()); ++i) {
            const auto& node = report.top_hotspots[i];
            std::cout << "  " << std::left << std::setw(32) << node.relative_path
                      << " " << std::right << std::setw(6) << std::fixed << std::setprecision(2)
                      << node.build_time.compilation_seconds << "s"
                      << "  Score: " << std::setprecision(1) << node.hotspot.total_score;
            if (!node.hotspot.score_breakdown.empty()) {
                std::cout << " (" << node.hotspot.score_breakdown << ")";
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";

    std::cout << BOLD << "DEPENDENCY HOTSPOTS" << RESET << "\n";
    if (report.top_fanin_headers.empty()) {
        std::cout << "  No high fan-in headers.\n";
    } else {
        for (size_t i = 0; i < std::min<size_t>(5, report.top_fanin_headers.size()); ++i) {
            const auto& node = report.top_fanin_headers[i];
            std::cout << "  " << std::left << std::setw(32) << node.relative_path
                      << " " << std::right << std::setw(4) << node.fan_stats.fan_in_transitive << " dependents\n";
        }
    }
    std::cout << "\n";

    std::cout << BOLD << "ARCHITECTURAL WARNINGS" << RESET << "\n";
    if (report.summary.circular_dependency_count > 0) {
        std::cout << "  " << RED << report.summary.circular_dependency_count << " circular dependencies" << RESET << "\n";
    } else {
        std::cout << "  " << GREEN << "0 circular dependencies" << RESET << "\n";
    }
    std::cout << "  " << (report.summary.high_fanout_header_count > 0 ? YELLOW : GREEN)
              << report.summary.high_fanout_header_count << " high fan-out headers" << RESET << "\n";
    std::cout << "  " << (report.summary.high_churn_complexity_count > 0 ? MAGENTA : GREEN)
              << report.summary.high_churn_complexity_count << " high-churn/high-complexity files" << RESET << "\n\n";

    std::cout << BOLD << "RECOMMENDATIONS & ACTION ITEMS" << RESET << "\n";
    if (report.recommendations.empty()) {
        std::cout << "  " << GREEN << "No critical recommendations. Clean build architecture!" << RESET << "\n";
    } else {
        for (const auto& rec : report.recommendations) {
            const char* sev_color = (rec.severity == Severity::High) ? RED : (rec.severity == Severity::Medium ? YELLOW : CYAN);
            std::cout << "  " << sev_color << std::left << std::setw(5) << severity_to_string(rec.severity) << RESET
                      << " " << rec.title << "\n";
            std::cout << "        -> " << rec.actionable_step << "\n";
        }
    }
    std::cout << "\n";
}

} // namespace compileforge
