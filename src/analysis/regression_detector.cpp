#include <compileforge/analysis/regression_detector.hpp>
#include <sstream>

namespace compileforge {

RegressionResult RegressionDetector::compare_reports(const JsonValue& base, const JsonValue& curr) {
    RegressionResult res;

    // Compare cycles
    size_t base_cycles = base["summary"]["circular_dependencies"].as_int();
    size_t curr_cycles = curr["summary"]["circular_dependencies"].as_int();

    if (curr_cycles > base_cycles) {
        res.is_regression = true;
        res.new_cycle_count = curr_cycles - base_cycles;
        std::ostringstream ss;
        ss << "+" << res.new_cycle_count << " new circular dependency loop(s)";
        res.regression_messages.push_back(ss.str());
    } else if (curr_cycles < base_cycles) {
        res.resolved_cycle_count = base_cycles - curr_cycles;
        std::ostringstream ss;
        ss << "-" << res.resolved_cycle_count << " circular dependency loop(s) resolved";
        res.improvement_messages.push_back(ss.str());
    }

    // Compare LOC
    long long base_loc = base["summary"]["sloc"].as_int();
    long long curr_loc = curr["summary"]["sloc"].as_int();
    res.loc_change = curr_loc - base_loc;

    if (res.loc_change > 0) {
        std::ostringstream ss;
        ss << "+" << res.loc_change << " SLOC added";
        res.improvement_messages.push_back(ss.str());
    } else if (res.loc_change < 0) {
        std::ostringstream ss;
        ss << res.loc_change << " SLOC reduced";
        res.improvement_messages.push_back(ss.str());
    }

    return res;
}

} // namespace compileforge
