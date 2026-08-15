#include "../test_framework.hpp"
#include "../../include/compileforge/validation/impact_validator.hpp"
#include "../../include/compileforge/validation/build_observer.hpp"

using namespace compileforge;

TEST_CASE(test_validation_precision_and_recall) {
    ImpactPrediction pred;
    pred.predicted_affected_tus = {"src/client.cpp", "src/server.cpp", "src/extra.cpp"};
    pred.predicted_rebuild_surface_pct = 30.0;
    pred.total_project_tus = 10;

    BuildObservation obs;
    obs.rebuilt_tus = {"src/client.cpp", "src/server.cpp", "src/other.cpp"};
    obs.observation_source = "BUILD_LOG";

    auto res = ImpactValidator::validate(pred, obs);

    // True Positives: client, server (2)
    // False Positives: extra (1)
    // False Negatives: other (1)
    ASSERT_EQ(res.true_positives, 2);
    ASSERT_EQ(res.false_positives, 1);
    ASSERT_EQ(res.false_negatives, 1);

    // Precision = 2 / 3 = 66.67%
    // Recall = 2 / 3 = 66.67%
    ASSERT_TRUE(res.precision >= 66.0 && res.precision <= 67.0);
    ASSERT_TRUE(res.recall >= 66.0 && res.recall <= 67.0);
}

TEST_CASE(test_build_log_parsing_ninja) {
    std::string ninja_log = R"(
[1/4] Building CXX object CMakeFiles/app.dir/src/main.cpp.o
[2/4] Building CXX object CMakeFiles/app.dir/src/network/client.cpp.o
[3/4] Linking CXX executable app
)";

    auto obs_res = BuildObserver::parse_build_log(ninja_log);
    ASSERT_TRUE(obs_res.is_ok());
    const auto& rebuilt = obs_res.value().rebuilt_tus;
    ASSERT_EQ(rebuilt.size(), 2);
}
