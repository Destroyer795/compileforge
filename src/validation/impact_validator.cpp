#include <compileforge/validation/impact_validator.hpp>
#include <compileforge/core/utils.hpp>
#include <unordered_set>
#include <cmath>
#include <algorithm>

namespace compileforge {

Result<ImpactPrediction> ImpactPrediction::from_json(const JsonValue& root) {
    if (!root.is_object()) {
        return Error(ErrorCode::ParseError, "Invalid prediction JSON object");
    }

    ImpactPrediction pred;
    if (root["rebuild_surface"].is_object()) {
        const auto& surf = root["rebuild_surface"];
        pred.predicted_rebuild_surface_pct = surf["percentage_tus_affected"].as_double();
        pred.total_project_tus = static_cast<size_t>(surf["total_affected_tus"].as_int());
        if (surf["affected_translation_units"].is_array()) {
            for (const auto& tu : surf["affected_translation_units"].as_array()) {
                if (tu.is_string()) pred.predicted_affected_tus.push_back(tu.as_string());
            }
        }
    }

    if (root["risk"].is_object()) {
        pred.predicted_risk_score = root["risk"]["total_risk_score"].as_int();
    }

    if (root["changed_files"].is_array()) {
        for (const auto& chg : root["changed_files"].as_array()) {
            if (chg.is_object() && chg["path"].is_string()) {
                pred.changed_files.push_back(chg["path"].as_string());
            }
        }
    }

    if (root["review_hotspots"].is_array()) {
        for (const auto& rh : root["review_hotspots"].as_array()) {
            if (rh.is_object() && rh["path"].is_string()) {
                std::string p = rh["path"].as_string();
                if (utils::ends_with(p, ".cpp") || utils::ends_with(p, ".cc") || utils::ends_with(p, ".cxx")) {
                    pred.predicted_affected_tus.push_back(p);
                } else {
                    pred.predicted_affected_headers.push_back(p);
                }
            }
        }
    }

    return pred;
}

JsonValue ImpactPrediction::to_json() const {
    JsonValue::ObjectType root;
    root["predicted_rebuild_surface_pct"] = predicted_rebuild_surface_pct;
    root["predicted_risk_score"] = predicted_risk_score;
    root["confidence"] = confidence;

    JsonValue::ArrayType tus;
    for (const auto& tu : predicted_affected_tus) tus.push_back(tu);
    root["predicted_affected_tus"] = tus;

    return root;
}

ImpactValidationResult ImpactValidator::validate(
    const ImpactPrediction& prediction,
    const BuildObservation& observation
) {
    ImpactValidationResult val;
    val.prediction = prediction;
    val.observation = observation;

    std::unordered_set<std::string> pred_set;
    for (const auto& tu : prediction.predicted_affected_tus) {
        pred_set.insert(utils::normalize_path(tu));
    }

    std::unordered_set<std::string> obs_set;
    for (const auto& tu : observation.rebuilt_tus) {
        obs_set.insert(utils::normalize_path(tu));
    }

    for (const auto& p : pred_set) {
        bool found = false;
        for (const auto& o : obs_set) {
            if (utils::path_equals(p, o) || utils::ends_with(o, p) || utils::ends_with(p, o)) {
                found = true;
                break;
            }
        }
        if (found) {
            val.true_positives++;
        } else {
            val.false_positives++;
        }
    }

    for (const auto& o : obs_set) {
        bool found = false;
        for (const auto& p : pred_set) {
            if (utils::path_equals(p, o) || utils::ends_with(o, p) || utils::ends_with(p, o)) {
                found = true;
                break;
            }
        }
        if (!found) {
            val.false_negatives++;
        }
    }

    size_t total_predicted = val.true_positives + val.false_positives;
    if (total_predicted > 0) {
        val.precision = (static_cast<double>(val.true_positives) / static_cast<double>(total_predicted)) * 100.0;
    } else {
        val.precision = 100.0;
    }

    size_t total_observed = val.true_positives + val.false_negatives;
    if (total_observed > 0) {
        val.recall = (static_cast<double>(val.true_positives) / static_cast<double>(total_observed)) * 100.0;
        val.recall_available = true;
    } else {
        val.recall = 100.0;
        val.recall_available = false;
    }

    double observed_rebuild_surface = 0.0;
    if (prediction.total_project_tus > 0) {
        observed_rebuild_surface = (static_cast<double>(total_observed) / static_cast<double>(prediction.total_project_tus)) * 100.0;
    }
    val.rebuild_surface_error_pct = std::abs(prediction.predicted_rebuild_surface_pct - observed_rebuild_surface);

    if (prediction.has_predicted_cost && observation.has_measured_duration && observation.actual_build_duration_seconds > 0.0) {
        val.build_cost_error_pct = std::abs(prediction.predicted_build_cost_seconds - observation.actual_build_duration_seconds) / observation.actual_build_duration_seconds * 100.0;
        val.build_cost_error_available = true;
    } else {
        val.build_cost_error_available = false;
    }

    if (val.precision >= 90.0 && val.rebuild_surface_error_pct <= 10.0) {
        val.overall_accuracy_rating = "EXCELLENT";
    } else if (val.precision >= 75.0) {
        val.overall_accuracy_rating = "GOOD";
    } else if (val.precision >= 50.0) {
        val.overall_accuracy_rating = "MODERATE";
    } else {
        val.overall_accuracy_rating = "LOW";
    }

    return val;
}

} // namespace compileforge
