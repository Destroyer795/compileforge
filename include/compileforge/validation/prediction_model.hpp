#ifndef COMPILEFORGE_VALIDATION_PREDICTION_MODEL_HPP
#define COMPILEFORGE_VALIDATION_PREDICTION_MODEL_HPP

#include <string>
#include <vector>
#include <compileforge/core/types.hpp>
#include <compileforge/impact/impact_analyzer.hpp>
#include <compileforge/impact/risk_scorer.hpp>
#include <compileforge/core/json.hpp>

namespace compileforge {

enum class MetricProvenance {
    Measured,
    Estimated,
    Observed,
    Inferred,
    Heuristic,
    Unavailable
};

inline std::string provenance_to_string(MetricProvenance p) {
    switch (p) {
        case MetricProvenance::Measured: return "MEASURED";
        case MetricProvenance::Estimated: return "ESTIMATED";
        case MetricProvenance::Observed: return "OBSERVED";
        case MetricProvenance::Inferred: return "INFERRED";
        case MetricProvenance::Heuristic: return "HEURISTIC";
        case MetricProvenance::Unavailable: return "UNAVAILABLE";
        default: return "ESTIMATED";
    }
}

struct ImpactPrediction {
    std::vector<std::string> changed_files;
    std::vector<std::string> predicted_affected_tus;
    std::vector<std::string> predicted_affected_headers;
    size_t total_project_tus{0};
    double predicted_rebuild_surface_pct{0.0};
    double predicted_build_cost_seconds{0.0};
    bool has_predicted_cost{false};
    int predicted_risk_score{0};
    std::string confidence{"MEDIUM"};

    static Result<ImpactPrediction> from_json(const JsonValue& root);
    JsonValue to_json() const;
};

struct BuildObservation {
    std::vector<std::string> rebuilt_tus;
    double actual_build_duration_seconds{0.0};
    bool has_measured_duration{false};
    size_t total_compiler_invocations{0};
    std::string observation_source{"BUILD_LOG"}; // BUILD_COMMAND, BUILD_LOG, DRY_RUN, UNKNOWN
};

struct ImpactValidationResult {
    ImpactPrediction prediction;
    BuildObservation observation;

    size_t true_positives{0};  // Predicted & actually rebuilt
    size_t false_positives{0}; // Predicted but not rebuilt
    size_t false_negatives{0}; // Rebuilt but not predicted

    double precision{0.0};     // TP / (TP + FP)
    double recall{0.0};        // TP / (TP + FN)
    bool recall_available{false};

    double rebuild_surface_error_pct{0.0}; // |Predicted % - Observed %|
    double build_cost_error_pct{0.0};
    bool build_cost_error_available{false};

    std::string overall_accuracy_rating{"GOOD"}; // EXCELLENT, GOOD, MODERATE, LOW
};

} // namespace compileforge

#endif // COMPILEFORGE_VALIDATION_PREDICTION_MODEL_HPP
