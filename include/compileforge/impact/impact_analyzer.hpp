#ifndef COMPILEFORGE_IMPACT_IMPACT_ANALYZER_HPP
#define COMPILEFORGE_IMPACT_IMPACT_ANALYZER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <compileforge/core/types.hpp>
#include <compileforge/git/git_analyzer.hpp>
#include <compileforge/graph/dependency_graph.hpp>

namespace compileforge {

struct AffectedNode {
    std::string relative_path;
    FileKind kind{FileKind::Header};
    size_t distance{0};
    bool is_translation_unit{false};
    bool is_hotspot{false};
    size_t sloc{0};
};

struct BuildCostEstimate {
    bool has_measured_timings{false};
    double estimated_compile_seconds{0.0};
    std::string confidence{"LOW"}; // LOW, MEDIUM, HIGH
    std::string basis_explanation;
};

struct ImpactAnalysisResult {
    std::vector<ChangedFileEntry> changed_files;
    std::vector<AffectedNode> affected_nodes;
    size_t total_affected_tus{0};
    size_t total_affected_headers{0};
    size_t total_affected_files{0};
    double percentage_tus_affected{0.0};
    double percentage_loc_affected{0.0};
    size_t max_impact_depth{0};
    std::string impact_classification{"LOW"}; // LOW, MEDIUM, HIGH, CRITICAL
    BuildCostEstimate cost_estimate;
};

class ImpactAnalyzer {
public:
    static ImpactAnalysisResult analyze_impact(
        const DependencyGraph& graph,
        const std::vector<ChangedFileEntry>& changed_files
    );
};

} // namespace compileforge

#endif // COMPILEFORGE_IMPACT_IMPACT_ANALYZER_HPP
