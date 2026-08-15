#include <compileforge/analysis/regression_detector.hpp>
#include <fstream>
#include <unordered_map>

namespace compileforge {

Result<RegressionReport> RegressionDetector::compare(const JsonValue& baseline_json, const JsonValue& current_json) {
    RegressionReport report;

    // 1. Compare circular dependencies count
    int baseline_cycles = baseline_json["summary"]["circular_dependencies"].as_int();
    int current_cycles = current_json["summary"]["circular_dependencies"].as_int();
    if (current_cycles > baseline_cycles) {
        report.has_regressions = true;
        RegressionDelta delta;
        delta.category = "CIRCULAR_DEPENDENCY";
        delta.message = "Introduced " + std::to_string(current_cycles - baseline_cycles) + " new circular dependency cycle(s)";
        delta.old_value = baseline_cycles;
        delta.new_value = current_cycles;
        report.deltas.push_back(delta);
    }

    // 2. Compare file hotspot scores
    std::unordered_map<std::string, double> base_scores;
    if (baseline_json["files"].is_array()) {
        for (const auto& item : baseline_json["files"].as_array()) {
            std::string path = item["path"].as_string();
            double score = item["hotspot_score"].as_double();
            base_scores[path] = score;
        }
    }

    if (current_json["files"].is_array()) {
        for (const auto& item : current_json["files"].as_array()) {
            std::string path = item["path"].as_string();
            double score = item["hotspot_score"].as_double();

            auto it = base_scores.find(path);
            if (it != base_scores.end()) {
                double diff = score - it->second;
                if (diff >= 5.0) { // 5 point hotspot score degradation limit
                    report.has_regressions = true;
                    RegressionDelta delta;
                    delta.category = "HOTSPOT_DEGRADATION";
                    delta.file = path;
                    delta.message = "Hotspot score degraded by " + std::to_string(diff) + " points";
                    delta.old_value = it->second;
                    delta.new_value = score;
                    report.deltas.push_back(delta);
                }
            }
        }
    }

    return report;
}

Result<RegressionReport> RegressionDetector::compare_files(const std::string& baseline_path, const std::string& current_path) {
    std::ifstream b_ifs(baseline_path);
    if (!b_ifs) return Error{ErrorCode::FileNotFound, "Could not open baseline report: " + baseline_path};
    std::string b_content((std::istreambuf_iterator<char>(b_ifs)), std::istreambuf_iterator<char>());
    auto b_json = JsonValue::parse(b_content);
    if (b_json.is_error()) return b_json.error();

    std::ifstream c_ifs(current_path);
    if (!c_ifs) return Error{ErrorCode::FileNotFound, "Could not open current report: " + current_path};
    std::string c_content((std::istreambuf_iterator<char>(c_ifs)), std::istreambuf_iterator<char>());
    auto c_json = JsonValue::parse(c_content);
    if (c_json.is_error()) return c_json.error();

    return compare(b_json.value(), c_json.value());
}

} // namespace compileforge
