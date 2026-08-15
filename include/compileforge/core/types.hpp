#ifndef COMPILEFORGE_CORE_TYPES_HPP
#define COMPILEFORGE_CORE_TYPES_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <cstdint>

namespace compileforge {

enum class FileKind {
    Header,
    TranslationUnit,
    Other
};

enum class IncludeKind {
    User,   // #include "file.hpp"
    System  // #include <file.hpp>
};

struct IncludeDirective {
    std::string raw_path;
    std::string resolved_path;
    IncludeKind kind{IncludeKind::User};
    size_t line_number{0};
    bool is_resolved{false};
    bool is_self_include{false};
    bool is_duplicate{false};
};

struct SourceMetricsData {
    size_t total_lines{0};
    size_t sloc{0}; // Source lines of code
    size_t comment_lines{0};
    size_t blank_lines{0};
    size_t cyclomatic_complexity{0};
    size_t macro_count{0};
    size_t template_count{0};
    bool has_pragma_once{false};
    bool has_header_guard{false};
    std::string header_guard_name;
};

struct FanStats {
    size_t fan_in_direct{0};        // Number of files directly including this header
    size_t fan_in_transitive{0};    // Total TUs/headers that transitively depend on this header
    size_t fan_out_direct{0};       // Number of headers directly included by this file
    size_t fan_out_transitive{0};   // Total unique headers transitively included by this file
    size_t impact_score{0};         // Weighted compilation blast radius
};

struct BuildTimeEstimate {
    double compilation_seconds{0.0};
    size_t preprocessed_bytes{0};
    size_t estimated_instantiations{0};
    bool is_measured{false}; // True if parsed from -ftime-trace
};

struct GitChurnData {
    size_t commit_count{0};
    size_t added_lines{0};
    size_t deleted_lines{0};
    size_t author_count{0};
    double active_age_days{0.0};
    bool git_tracked{false};
};

struct HotspotScore {
    double total_score{0.0}; // 0.0 to 100.0
    double build_time_factor{0.0};
    double fan_in_factor{0.0};
    double complexity_factor{0.0};
    double churn_factor{0.0};
    std::string score_breakdown;
};

enum class Severity {
    High,
    Medium,
    Low
};

inline std::string severity_to_string(Severity sev) {
    switch (sev) {
        case Severity::High: return "HIGH";
        case Severity::Medium: return "MED";
        case Severity::Low: return "LOW";
    }
    return "LOW";
}

struct Recommendation {
    std::string id;
    Severity severity{Severity::Low};
    std::string target_file;
    std::string title;
    std::string description;
    std::string actionable_step;
};

struct CompileCommandEntry {
    std::string directory;
    std::string command;
    std::string file;
    std::string output;
    std::vector<std::string> arguments;
    std::vector<std::string> include_dirs;
    std::vector<std::string> defines;
    std::vector<std::string> forced_includes;
};

struct FileNode {
    std::string relative_path;
    std::string canonical_path;
    FileKind kind{FileKind::Other};
    uint64_t file_size_bytes{0};
    std::string content_hash;
    
    std::vector<IncludeDirective> includes;
    SourceMetricsData metrics;
    FanStats fan_stats;
    BuildTimeEstimate build_time;
    GitChurnData git_data;
    HotspotScore hotspot;
};

struct DependencyCycle {
    std::vector<std::string> cycle_path; // e.g. ["A.hpp", "B.hpp", "C.hpp", "A.hpp"]
    size_t length{0};
};

struct ProjectSummary {
    size_t total_files{0};
    size_t total_headers{0};
    size_t total_translation_units{0};
    size_t total_loc{0};
    size_t total_sloc{0};
    double total_estimated_build_seconds{0.0};
    size_t circular_dependency_count{0};
    size_t high_fanout_header_count{0};
    size_t high_churn_complexity_count{0};
};

} // namespace compileforge

#endif // COMPILEFORGE_CORE_TYPES_HPP
