#include <tests/test_framework.hpp>
#include <compileforge/analysis/regression_detector.hpp>

using namespace compileforge;

TEST_CASE(test_regression_detector_comparison) {
    std::string baseline_json = R"({
        "summary": { "circular_dependencies": 0, "sloc": 100 }
    })";

    std::string current_json = R"({
        "summary": { "circular_dependencies": 1, "sloc": 120 }
    })";

    auto b_json = JsonValue::parse(baseline_json);
    auto c_json = JsonValue::parse(current_json);

    ASSERT_TRUE(b_json.is_ok());
    ASSERT_TRUE(c_json.is_ok());

    auto reg_res = RegressionDetector::compare_reports(b_json.value(), c_json.value());
    ASSERT_TRUE(reg_res.is_regression);
    ASSERT_EQ(reg_res.new_cycle_count, 1);
}
