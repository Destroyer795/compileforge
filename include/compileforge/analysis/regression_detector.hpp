#ifndef COMPILEFORGE_ANALYSIS_REGRESSION_DETECTOR_HPP
#define COMPILEFORGE_ANALYSIS_REGRESSION_DETECTOR_HPP

#include <string>
#include <vector>
#include <compileforge/core/result.hpp>
#include <compileforge/core/json.hpp>

namespace compileforge {

struct RegressionDelta {
    std::string category;
    std::string file;
    std::string message;
    double old_value{0.0};
    double new_value{0.0};
};

struct RegressionReport {
    bool has_regressions{false};
    std::vector<RegressionDelta> deltas;
};

class RegressionDetector {
public:
    static Result<RegressionReport> compare(const JsonValue& baseline_json, const JsonValue& current_json);
    static Result<RegressionReport> compare_files(const std::string& baseline_path, const std::string& current_path);
};

} // namespace compileforge

#endif // COMPILEFORGE_ANALYSIS_REGRESSION_DETECTOR_HPP
