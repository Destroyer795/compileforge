#include "../../include/compileforge/build/build_trace.hpp"
#include "../../include/compileforge/core/json.hpp"
#include <fstream>

namespace compileforge {

Result<std::unordered_map<std::string, double>> BuildTraceAnalyzer::load_ftime_trace_file(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs) {
        return Error{
            ErrorCode::FileNotFound,
            "Could not open ftime-trace file: " + filepath,
            "BuildTraceAnalyzer::load_ftime_trace_file"
        };
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto parse_res = JsonValue::parse(content);
    if (parse_res.is_error()) return parse_res.error();

    std::unordered_map<std::string, double> timing_map;
    const auto& root = parse_res.value();

    if (root["traceEvents"].is_array()) {
        for (const auto& ev : root["traceEvents"].as_array()) {
            std::string name = ev["name"].as_string();
            double dur_us = ev["dur"].as_double(); // microseconds in ftime-trace
            if (name == "ExecuteCompiler" || name == "Frontend" || name == "Source") {
                std::string detail = ev["args"]["detail"].as_string();
                if (!detail.empty()) {
                    timing_map[detail] += dur_us / 1e6; // convert to seconds
                }
            }
        }
    }

    return timing_map;
}

void BuildTraceAnalyzer::estimate_build_times(DependencyGraph& graph) {
    for (const auto& path : graph.all_nodes()) {
        auto* node = graph.get_node_mut(path);
        if (!node) continue;

        if (node->build_time.is_measured) continue;

        // Calculate total transitive preprocessed lines
        size_t total_preprocessed_loc = node->metrics.total_lines;
        auto transitive_deps = graph.get_transitive_dependencies(path);

        for (const auto& dep_path : transitive_deps) {
            const auto* dep_node = graph.get_node(dep_path);
            if (dep_node) {
                total_preprocessed_loc += dep_node->metrics.total_lines;
            }
        }

        // Heuristic formula: Base cost per 10k lines = 0.5s + template multiplier + complexity
        double base_seconds = static_cast<double>(total_preprocessed_loc) * 0.00015;
        double template_penalty = static_cast<double>(node->metrics.template_count) * 0.05;
        double complexity_penalty = static_cast<double>(node->metrics.cyclomatic_complexity) * 0.01;

        node->build_time.compilation_seconds = base_seconds + template_penalty + complexity_penalty;
        node->build_time.preprocessed_bytes = total_preprocessed_loc * 35; // ~35 bytes/line
    }
}

} // namespace compileforge
