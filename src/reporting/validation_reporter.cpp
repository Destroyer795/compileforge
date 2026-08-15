#include <compileforge/reporting/validation_reporter.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>

namespace compileforge {

void ValidationReporter::print_terminal(const ImpactValidationResult& res) {
    std::cout << "\n==========================================================\n";
    std::cout << "             CHANGE IMPACT PREDICTION VALIDATION          \n";
    std::cout << "==========================================================\n\n";

    std::cout << "PREDICTION VS OBSERVATION\n";
    std::cout << "  Predicted Affected TUs:      " << res.prediction.predicted_affected_tus.size() << " [ESTIMATED]\n";
    std::cout << "  Observed Rebuilt TUs:        " << res.observation.rebuilt_tus.size() << " [OBSERVED]\n";
    std::cout << "  True Positives (Correct):    " << res.true_positives << "\n";
    std::cout << "  False Positives (Over-pred): " << res.false_positives << "\n";
    std::cout << "  False Negatives (Missed):    " << res.false_negatives << "\n\n";

    std::cout << "ACCURACY METRICS\n";
    std::cout << "  Prediction Precision:        " << std::fixed << std::setprecision(1) << res.precision << "%\n";
    if (res.recall_available) {
        std::cout << "  Prediction Recall:           " << std::fixed << std::setprecision(1) << res.recall << "%\n";
    } else {
        std::cout << "  Prediction Recall:           UNAVAILABLE (No rebuilt TUs observed)\n";
    }

    std::cout << "  Rebuild Surface Error Delta: " << std::fixed << std::setprecision(1) << res.rebuild_surface_error_pct << "%\n";

    if (res.build_cost_error_available) {
        std::cout << "  Build-Cost Error Delta:      " << std::fixed << std::setprecision(1) << res.build_cost_error_pct << "%\n";
    } else {
        std::cout << "  Build-Cost Error Delta:      UNAVAILABLE (No historical build timings)\n";
    }

    std::cout << "  Overall Accuracy Rating:     " << res.overall_accuracy_rating << "\n\n";
    std::cout << "==========================================================\n";
}

JsonValue ValidationReporter::to_json(const ImpactValidationResult& res) {
    JsonValue::ObjectType root;
    root["schema_version"] = "1.0_validation";

    JsonValue::ObjectType pred_obj;
    pred_obj["predicted_affected_tus_count"] = static_cast<int>(res.prediction.predicted_affected_tus.size());
    pred_obj["predicted_rebuild_surface_pct"] = res.prediction.predicted_rebuild_surface_pct;
    pred_obj["confidence"] = res.prediction.confidence;
    root["prediction"] = pred_obj;

    JsonValue::ObjectType obs_obj;
    obs_obj["observed_rebuilt_tus_count"] = static_cast<int>(res.observation.rebuilt_tus.size());
    obs_obj["source"] = res.observation.observation_source;
    obs_obj["duration_seconds"] = res.observation.actual_build_duration_seconds;
    root["observation"] = obs_obj;

    JsonValue::ObjectType acc_obj;
    acc_obj["true_positives"] = static_cast<int>(res.true_positives);
    acc_obj["false_positives"] = static_cast<int>(res.false_positives);
    acc_obj["false_negatives"] = static_cast<int>(res.false_negatives);
    acc_obj["precision"] = res.precision;
    acc_obj["recall"] = res.recall;
    acc_obj["recall_available"] = res.recall_available;
    acc_obj["rebuild_surface_error_pct"] = res.rebuild_surface_error_pct;
    acc_obj["overall_accuracy_rating"] = res.overall_accuracy_rating;
    root["accuracy"] = acc_obj;

    return root;
}

std::string ValidationReporter::render_json(const ImpactValidationResult& res, int indent) {
    return to_json(res).serialize(indent);
}

std::string ValidationReporter::render_html(const ImpactValidationResult& res, const std::string& project_name) {
    std::ostringstream html;
    html << R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <title>CompileForge Validation - )HTML" << project_name << R"HTML(</title>
  <style>
    :root { --bg: #0d1117; --card-bg: #161b22; --border: #30363d; --text: #c9d1d9; --accent: #58a6ff; --success: #3fb950; --warning: #d29922; }
    body { font-family: sans-serif; background: var(--bg); color: var(--text); padding: 30px; }
    .card { background: var(--card-bg); border: 1px solid var(--border); border-radius: 8px; padding: 20px; margin-bottom: 20px; }
    .val { font-size: 28px; font-weight: bold; color: #fff; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 15px; }
  </style>
</head>
<body>
  <h1>Prediction Validation: )HTML" << project_name << R"HTML(</h1>
  <div class="grid">
    <div class="card"><small>Precision</small><div class="val">)HTML" << std::fixed << std::setprecision(1) << res.precision << R"HTML(%</div></div>
    <div class="card"><small>True Positives</small><div class="val">)HTML" << res.true_positives << R"HTML(</div></div>
    <div class="card"><small>Rebuilt TUs</small><div class="val">)HTML" << res.observation.rebuilt_tus.size() << R"HTML(</div></div>
    <div class="card"><small>Accuracy Rating</small><div class="val" style="color: var(--success);">)HTML" << res.overall_accuracy_rating << R"HTML(</div></div>
  </div>
</body>
</html>)HTML";
    return html.str();
}

} // namespace compileforge
