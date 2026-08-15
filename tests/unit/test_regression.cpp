#include <tests/test_framework.hpp>
#include <compileforge/analysis/regression_detector.hpp>

using namespace compileforge;

TEST_CASE(test_regression_detector_comparison) {
    std::string base_str = R"({
        "summary": { "circular_dependencies": 0 },
        "files": [
            { "path": "src/render.cpp", "hotspot_score": 20.0 }
        ]
    })";

    std::string curr_str = R"({
        "summary": { "circular_dependencies": 2 },
        "files": [
            { "path": "src/render.cpp", "hotspot_score": 35.0 }
        ]
    })";

    auto b_json = JsonValue::parse(base_str);
    auto c_json = JsonValue::parse(curr_str);

    ASSERT_TRUE(b_json.has_value());
    ASSERT_TRUE(c_json.has_value());

    auto reg_res = RegressionDetector::compare(b_json.value(), c_json.value());
    ASSERT_TRUE(reg_res.has_value());

    const auto& report = reg_res.value();
    ASSERT_TRUE(report.has_regressions);
    ASSERT_EQ(report.deltas.size(), 2);
}
