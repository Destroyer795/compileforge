#include <compileforge/reporting/validation_reporter.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace compileforge {

void ValidationReporter::print_terminal(const ImpactValidationResult& res) {
    bool use_colors = true;
    const char* RESET = use_colors ? "\033[0m" : "";
    const char* BOLD = use_colors ? "\033[1m" : "";
    const char* GREEN = use_colors ? "\033[32m" : "";
    const char* RED = use_colors ? "\033[31m" : "";
    const char* YELLOW = use_colors ? "\033[33m" : "";
    const char* CYAN = use_colors ? "\033[36m" : "";

    std::cout << "\n==========================================================\n";
    std::cout << "             CHANGE IMPACT PREDICTION VALIDATION          \n";
    std::cout << "==========================================================\n\n";

    std::cout << "PREDICTION VS OBSERVATION\n";
    std::cout << "  Predicted Affected TUs:      " << res.prediction.predicted_affected_tus.size() << " [ESTIMATED]\n";
    std::cout << "  Observed Rebuilt TUs:        " << res.observation.rebuilt_tus.size() << " [OBSERVED]\n";
    std::cout << "  True Positives (Correct):    " << GREEN << res.true_positives << RESET << "\n";
    std::cout << "  False Positives (Over-pred): " << YELLOW << res.false_positives << RESET << "\n";
    std::cout << "  False Negatives (Missed):    " << RED << res.false_negatives << RESET << "\n\n";

    std::cout << "ACCURACY METRICS\n";
    if (res.precision_available) {
        std::cout << "  Prediction Precision:        " << std::fixed << std::setprecision(1) << res.precision << "%\n";
    } else {
        std::cout << "  Prediction Precision:        UNAVAILABLE (No predicted TUs)\n";
    }

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

    std::cout << "  Overall Accuracy Rating:     " << BOLD << CYAN << res.overall_accuracy_rating << RESET << "\n\n";
    std::cout << "==========================================================\n\n";
}

JsonValue ValidationReporter::to_json(const ImpactValidationResult& res) {
    JsonValue::ObjectType root;

    root["schema_version"] = "1.0_validation";

    // Observation
    JsonValue::ObjectType obs_obj;
    obs_obj["source"] = res.observation.observation_source;
    obs_obj["duration_seconds"] = res.observation.actual_build_duration_seconds;
    obs_obj["total_compiler_invocations"] = static_cast<int>(res.observation.total_compiler_invocations);

    JsonValue::ArrayType reb_arr;
    for (const auto& tu : res.observation.rebuilt_tus) {
        reb_arr.push_back(tu);
    }
    obs_obj["rebuilt_tus"] = reb_arr;
    root["observation"] = obs_obj;

    // Accuracy
    JsonValue::ObjectType acc_obj;
    acc_obj["true_positives"] = static_cast<int>(res.true_positives);
    acc_obj["false_positives"] = static_cast<int>(res.false_positives);
    acc_obj["false_negatives"] = static_cast<int>(res.false_negatives);
    acc_obj["precision"] = res.precision;
    acc_obj["precision_available"] = res.precision_available;
    acc_obj["recall"] = res.recall;
    acc_obj["recall_available"] = res.recall_available;
    acc_obj["rebuild_surface_error_pct"] = res.rebuild_surface_error_pct;
    acc_obj["build_cost_error_pct"] = res.build_cost_error_pct;
    acc_obj["build_cost_error_available"] = res.build_cost_error_available;
    acc_obj["overall_accuracy_rating"] = res.overall_accuracy_rating;
    root["accuracy"] = acc_obj;

    return root;
}

std::string ValidationReporter::render_json(const ImpactValidationResult& res, int indent) {
    return to_json(res).serialize(indent);
}

std::string ValidationReporter::render_html(const ImpactValidationResult& res, const std::string& project_name) {
    std::ostringstream html;

    std::string rating_color = "#4d6b42"; // olive
    if (res.overall_accuracy_rating == "LOW") rating_color = "#82261e";
    else if (res.overall_accuracy_rating == "MODERATE") rating_color = "#b0802c";

    html << R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>CompileForge Prediction Validation &mdash; )HTML" << project_name << R"HTML(</title>
  <style>
    :root {
      --bg-parchment: #f7f4ed;
      --surface-card: #fdfbf7;
      --text-ink: #24211e;
      --text-muted: #6b635b;
      --text-faint: #9c9489;
      --border-hairline: #dfd8cb;
      --border-dark: #24211e;
      --accent-olive: #4d6b42;
      --accent-mustard: #b0802c;
      --accent-orange: #b84c24;
      --accent-crimson: #82261e;
      --accent-teal: #2e6065;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      background-color: var(--bg-parchment);
      color: var(--text-ink);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", sans-serif;
      line-height: 1.6;
      padding: 40px 24px;
      -webkit-font-smoothing: antialiased;
    }
    .container { max-width: 1040px; margin: 0 auto; }
    
    /* Editorial Masthead */
    .masthead {
      border-bottom: 2px solid var(--border-dark);
      padding-bottom: 20px;
      margin-bottom: 32px;
      display: flex;
      justify-content: space-between;
      align-items: flex-end;
      flex-wrap: wrap;
      gap: 16px;
    }
    .brand-eyebrow {
      font-family: "SF Mono", "Consolas", "Liberation Mono", "Menlo", monospace;
      font-size: 11px;
      letter-spacing: 0.16em;
      text-transform: uppercase;
      color: var(--accent-teal);
      font-weight: 700;
      margin-bottom: 6px;
    }
    .report-title {
      font-family: "Newsreader", "Georgia", "Baskerville", "Times New Roman", serif;
      font-size: 36px;
      font-weight: 700;
      letter-spacing: -0.02em;
      color: var(--text-ink);
      line-height: 1.1;
    }
    .meta-col {
      font-family: "SF Mono", "Consolas", "Liberation Mono", "Menlo", monospace;
      font-size: 12px;
      color: var(--text-muted);
      text-align: right;
      line-height: 1.5;
    }
    .meta-tag {
      display: inline-block;
      border: 1px solid var(--border-hairline);
      background: var(--surface-card);
      padding: 3px 8px;
      margin-top: 4px;
      font-weight: 600;
      color: var(--text-ink);
    }

    /* Typographic Metric Strip */
    .metric-strip {
      display: grid;
      grid-template-columns: repeat(4, 1fr);
      background: var(--surface-card);
      border: 1px solid var(--border-hairline);
      border-top: 3px solid var(--border-dark);
      margin-bottom: 36px;
    }
    .metric-cell {
      padding: 20px;
      border-right: 1px solid var(--border-hairline);
    }
    .metric-cell:last-child { border-right: none; }
    .metric-label {
      font-family: "SF Mono", "Consolas", "Liberation Mono", "Menlo", monospace;
      font-size: 11px;
      letter-spacing: 0.12em;
      text-transform: uppercase;
      color: var(--text-muted);
      margin-bottom: 8px;
    }
    .metric-val {
      font-family: "Newsreader", "Georgia", "Baskerville", serif;
      font-size: 36px;
      font-weight: 700;
      line-height: 1;
      color: var(--text-ink);
    }
    .metric-unit {
      font-family: "SF Mono", "Consolas", "Liberation Mono", monospace;
      font-size: 14px;
      font-weight: 400;
      color: var(--text-muted);
      margin-left: 2px;
    }

    /* Two-Column Comparison */
    .comparison-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 24px;
      margin-bottom: 36px;
    }
    .comparison-panel {
      background: var(--surface-card);
      border: 1px solid var(--border-hairline);
      padding: 24px;
    }
    .panel-title {
      font-family: "Newsreader", "Georgia", "Baskerville", serif;
      font-size: 20px;
      font-weight: 700;
      color: var(--text-ink);
      margin-bottom: 16px;
      border-bottom: 1px solid var(--border-hairline);
      padding-bottom: 8px;
    }
    .comparison-row {
      display: flex;
      justify-content: space-between;
      padding: 10px 0;
      border-bottom: 1px solid var(--border-hairline);
      font-size: 14px;
    }
    .comparison-row:last-child { border-bottom: none; }
    .row-label { color: var(--text-muted); }
    .row-val {
      font-family: "SF Mono", "Consolas", monospace;
      font-weight: 600;
      color: var(--text-ink);
    }

    /* Technical Data Table */
    table.editorial-table {
      width: 100%;
      border-collapse: collapse;
      background: var(--surface-card);
      border: 1px solid var(--border-hairline);
      margin-bottom: 36px;
    }
    table.editorial-table th {
      font-family: "SF Mono", "Consolas", monospace;
      font-size: 11px;
      letter-spacing: 0.12em;
      text-transform: uppercase;
      color: var(--text-muted);
      text-align: left;
      padding: 12px 16px;
      background: #eee8dc;
      border-bottom: 1px solid var(--border-hairline);
    }
    table.editorial-table td {
      padding: 12px 16px;
      border-bottom: 1px solid var(--border-hairline);
      font-size: 14px;
      color: var(--text-ink);
    }
    table.editorial-table tr:last-child td { border-bottom: none; }
    .path-mono {
      font-family: "SF Mono", "Consolas", "Liberation Mono", monospace;
      font-size: 13px;
    }
    .tag-editorial {
      font-family: "SF Mono", "Consolas", monospace;
      font-size: 11px;
      font-weight: 700;
      padding: 2px 6px;
      border: 1px solid var(--border-hairline);
    }
    .tag-correct { background: #eaf2e8; color: var(--accent-olive); border-color: #cde0ca; }

    /* Technical Note / Methodology Callout */
    .methodology-box {
      border-left: 3px solid var(--accent-teal);
      background: var(--surface-card);
      padding: 16px 20px;
      border-top: 1px solid var(--border-hairline);
      border-right: 1px solid var(--border-hairline);
      border-bottom: 1px solid var(--border-hairline);
      margin-bottom: 40px;
      font-size: 13px;
      color: var(--text-muted);
      line-height: 1.6;
    }
    .methodology-title {
      font-family: "SF Mono", "Consolas", monospace;
      font-size: 11px;
      letter-spacing: 0.12em;
      text-transform: uppercase;
      color: var(--accent-teal);
      font-weight: 700;
      margin-bottom: 4px;
    }

    /* Minimal Colophon Footer */
    footer.colophon {
      border-top: 1px solid var(--border-hairline);
      padding-top: 20px;
      display: flex;
      justify-content: space-between;
      font-family: "SF Mono", "Consolas", monospace;
      font-size: 11px;
      color: var(--text-faint);
    }

    @media (max-width: 768px) {
      .metric-strip { grid-template-columns: repeat(2, 1fr); }
      .metric-cell:nth-child(2) { border-right: none; }
      .comparison-grid { grid-template-columns: 1fr; }
      .masthead { flex-direction: column; align-items: flex-start; }
      .meta-col { text-align: left; }
    }
  </style>
</head>
<body>
  <div class="container">
    <header class="masthead">
      <div>
        <div class="brand-eyebrow">CompileForge // Prediction Validation</div>
        <h1 class="report-title">Change-Impact Verification</h1>
      </div>
      <div class="meta-col">
        <div>Target: <strong>)HTML" << project_name << R"HTML(</strong></div>
        <div class="meta-tag">Observed Build Verification</div>
      </div>
    </header>

    <div class="metric-strip">
      <div class="metric-cell">
        <div class="metric-label">Precision</div>
        <div class="metric-val" style="color: )HTML" << (res.precision_available ? "var(--text-ink)" : "var(--text-faint)") << R"HTML(;">)HTML";
    if (res.precision_available) {
        html << std::fixed << std::setprecision(1) << res.precision << "<span class=\"metric-unit\">%</span>";
    } else {
        html << "<span style=\"font-size:20px;\">N/A</span>";
    }
    html << R"HTML(</div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Recall</div>
        <div class="metric-val" style="color: )HTML" << (res.recall_available ? "var(--text-ink)" : "var(--text-faint)") << R"HTML(;">)HTML";
    if (res.recall_available) {
        html << std::fixed << std::setprecision(1) << res.recall << "<span class=\"metric-unit\">%</span>";
    } else {
        html << "<span style=\"font-size:20px;\">N/A</span>";
    }
    html << R"HTML(</div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Surface Error Delta</div>
        <div class="metric-val">)HTML" << std::fixed << std::setprecision(1) << res.rebuild_surface_error_pct << R"HTML(<span class="metric-unit">%</span></div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Accuracy Rating</div>
        <div class="metric-val" style="color: )HTML" << rating_color << R"HTML(;">)HTML" << res.overall_accuracy_rating << R"HTML(</div>
      </div>
    </div>

    <div class="comparison-grid">
      <div class="comparison-panel">
        <div class="panel-title">Predicted Impact Model</div>
        <div class="comparison-row">
          <span class="row-label">Predicted Affected TUs</span>
          <span class="row-val">)HTML" << res.prediction.predicted_affected_tus.size() << R"HTML(</span>
        </div>
        <div class="comparison-row">
          <span class="row-label">Predicted Surface</span>
          <span class="row-val">)HTML" << std::fixed << std::setprecision(1) << res.prediction.predicted_rebuild_surface_pct << R"HTML(%</span>
        </div>
        <div class="comparison-row">
          <span class="row-label">Predicted Risk Score</span>
          <span class="row-val">)HTML" << res.prediction.predicted_risk_score << R"HTML( / 100</span>
        </div>
      </div>

      <div class="comparison-panel">
        <div class="panel-title">Observed Compiler Activity</div>
        <div class="comparison-row">
          <span class="row-label">Actually Rebuilt TUs</span>
          <span class="row-val">)HTML" << res.observation.rebuilt_tus.size() << R"HTML(</span>
        </div>
        <div class="comparison-row">
          <span class="row-label">Compiler Invocations</span>
          <span class="row-val">)HTML" << res.observation.total_compiler_invocations << R"HTML(</span>
        </div>
        <div class="comparison-row">
          <span class="row-label">Observation Source</span>
          <span class="row-val">)HTML" << res.observation.observation_source << R"HTML(</span>
        </div>
      </div>
    </div>

    <div class="comparison-panel" style="margin-bottom: 36px;">
      <div class="panel-title">Classification Matrix</div>
      <div class="comparison-row">
        <span class="row-label">True Positives (Correctly Predicted Rebuilds)</span>
        <span class="row-val" style="color: var(--accent-olive);">)HTML" << res.true_positives << R"HTML(</span>
      </div>
      <div class="comparison-row">
        <span class="row-label">False Positives (Over-Predicted / Skipped)</span>
        <span class="row-val" style="color: var(--accent-mustard);">)HTML" << res.false_positives << R"HTML(</span>
      </div>
      <div class="comparison-row">
        <span class="row-label">False Negatives (Missed Rebuilds)</span>
        <span class="row-val" style="color: var(--accent-crimson);">)HTML" << res.false_negatives << R"HTML(</span>
      </div>
    </div>

    <table class="editorial-table">
      <thead>
        <tr>
          <th style="width: 75%;">Observed Rebuilt Translation Unit</th>
          <th style="width: 25%;">Status</th>
        </tr>
      </thead>
      <tbody>
)HTML";

    if (res.observation.rebuilt_tus.empty()) {
        html << "        <tr><td colspan=\"2\" style=\"color: var(--text-muted); font-style: italic;\">No translation units were observed to rebuild.</td></tr>\n";
    } else {
        for (const auto& tu : res.observation.rebuilt_tus) {
            html << "        <tr>\n";
            html << "          <td class=\"path-mono\">" << tu << "</td>\n";
            html << "          <td><span class=\"tag-editorial tag-correct\">REBUILT</span></td>\n";
            html << "        </tr>\n";
        }
    }

    html << R"HTML(
      </tbody>
    </table>

    <div class="methodology-box">
      <div class="methodology-title">Validation Protocol</div>
      Validation compares statically predicted affected translation units against compiler invocations captured from build logs or live executions. Accuracy ratings and precision/recall metrics reflect alignment between static preprocessor reachability and build system execution.
    </div>

    <footer class="colophon">
      <div>CompileForge &middot; Native C++20 Build Intelligence</div>
      <div>Zero Third-Party Runtime Dependencies &middot; MIT License</div>
    </footer>
  </div>
</body>
</html>)HTML";

    return html.str();
}

} // namespace compileforge
