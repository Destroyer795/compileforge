#include <compileforge/recommendations/recommendation_engine.hpp>
#include <sstream>
#include <algorithm>

namespace compileforge {

std::vector<Recommendation> RecommendationEngine::generate_recommendations(
    const DependencyGraph& graph,
    const std::vector<DependencyCycle>& cycles
) {
    std::vector<Recommendation> recs;
    size_t rec_counter = 1;

    // Rule 1: Circular dependencies (HIGH)
    for (const auto& cycle : cycles) {
        Recommendation rec;
        rec.id = "REC-CYCLE-" + std::to_string(rec_counter++);
        rec.severity = Severity::High;
        rec.target_file = cycle.cycle_path.empty() ? "" : cycle.cycle_path[0];
        rec.title = "Fix circular dependency loop (" + std::to_string(cycle.length) + " files)";

        std::ostringstream ss;
        ss << "Include cycle detected: ";
        for (size_t i = 0; i < cycle.cycle_path.size(); ++i) {
            ss << cycle.cycle_path[i];
            if (i + 1 < cycle.cycle_path.size()) ss << " -> ";
        }
        rec.description = ss.str();
        rec.actionable_step = "Use forward declarations or move shared declarations into a standalone header to break the cycle.";
        recs.push_back(std::move(rec));
    }

    for (const auto& path : graph.all_nodes()) {
        const auto* node = graph.get_node(path);
        if (!node) continue;

        // Rule 2: Heavyweight header with high fan-in (HIGH)
        if (node->kind == FileKind::Header && node->fan_stats.fan_in_transitive > 10 && node->metrics.total_lines > 250) {
            Recommendation rec;
            rec.id = "REC-HEADER-" + std::to_string(rec_counter++);
            rec.severity = Severity::High;
            rec.target_file = node->relative_path;
            rec.title = "Split heavyweight header: " + node->relative_path;
            rec.description = node->relative_path + " has " + std::to_string(node->fan_stats.fan_in_transitive) +
                             " dependents and " + std::to_string(node->metrics.total_lines) + " lines of code.";
            rec.actionable_step = "Extract template specializations or implementation details into a separate _impl.hpp or .cpp file.";
            recs.push_back(std::move(rec));
        }

        // Rule 3: High Churn & High Complexity (HIGH)
        if (node->git_data.commit_count > 10 && node->metrics.cyclomatic_complexity > 15) {
            Recommendation rec;
            rec.id = "REC-CHURN-" + std::to_string(rec_counter++);
            rec.severity = Severity::High;
            rec.target_file = node->relative_path;
            rec.title = "Refactor high-churn/high-complexity module: " + node->relative_path;
            rec.description = node->relative_path + " has high commit frequency (" + std::to_string(node->git_data.commit_count) +
                             " commits) and high complexity (" + std::to_string(node->metrics.cyclomatic_complexity) + ").";
            rec.actionable_step = "Decompose large functions and extract reusable component classes to reduce regression risk.";
            recs.push_back(std::move(rec));
        }

        // Rule 4: High fan-out file (MED)
        if (node->fan_stats.fan_out_direct > 15) {
            Recommendation rec;
            rec.id = "REC-FANOUT-" + std::to_string(rec_counter++);
            rec.severity = Severity::Medium;
            rec.target_file = node->relative_path;
            rec.title = "Reduce transitive includes in: " + node->relative_path;
            rec.description = node->relative_path + " directly includes " + std::to_string(node->fan_stats.fan_out_direct) + " headers.";
            rec.actionable_step = "Audit direct include list and remove unused header inclusions.";
            recs.push_back(std::move(rec));
        }

        // Rule 5: Missing header guard / #pragma once (MED)
        if (node->kind == FileKind::Header && !node->metrics.has_pragma_once && !node->metrics.has_header_guard) {
            Recommendation rec;
            rec.id = "REC-GUARD-" + std::to_string(rec_counter++);
            rec.severity = Severity::Medium;
            rec.target_file = node->relative_path;
            rec.title = "Add header guard or #pragma once: " + node->relative_path;
            rec.description = node->relative_path + " lacks a standard include guard or #pragma once directive.";
            rec.actionable_step = "Add '#pragma once' to the top of the header file.";
            recs.push_back(std::move(rec));
        }
    }

    return recs;
}

} // namespace compileforge
