#ifndef COMPILEFORGE_ANALYSIS_REGRESSION_DETECTOR_HPP
#define COMPILEFORGE_ANALYSIS_REGRESSION_DETECTOR_HPP

#include <string>
#include <vector>
#include "../core/json.hpp"

namespace compileforge {

struct RegressionResult {
    bool is_regression{false};
    int health_score_before{100};
    int health_score_after{100};
    int health_score_change{0};
    size_t new_cycle_count{0};
    size_t resolved_cycle_count{0};
    std::vector<std::string> new_hotspots;
    std::vector<std::string> resolved_hotspots;
    long long loc_change{0};
    std::vector<std::string> regression_messages;
    std::vector<std::string> improvement_messages;
};

class RegressionDetector {
public:
    static RegressionResult compare_reports(const JsonValue& baseline_json, const JsonValue& current_json);
};

} // namespace compileforge

#endif // COMPILEFORGE_ANALYSIS_REGRESSION_DETECTOR_HPP
