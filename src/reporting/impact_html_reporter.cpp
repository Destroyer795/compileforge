#include <compileforge/reporting/impact_html_reporter.hpp>
#include <sstream>
#include <iomanip>

namespace compileforge {

JsonValue ImpactJsonReporter::to_json(const ImpactReport& report) {
    JsonValue::ObjectType root;

    root["schema_version"] = "1.0_impact";

    // Risk
    JsonValue::ObjectType risk_obj;
    risk_obj["total_risk_score"] = report.risk.score_breakdown.total_risk_score;
    risk_obj["impact_factor"] = report.risk.score_breakdown.impact_factor;
    risk_obj["build_cost_factor"] = report.risk.score_breakdown.build_cost_factor;
    risk_obj["architecture_factor"] = report.risk.score_breakdown.architecture_factor;
    risk_obj["git_churn_factor"] = report.risk.score_breakdown.git_churn_factor;
    risk_obj["complexity_factor"] = report.risk.score_breakdown.complexity_factor;
    risk_obj["cycle_factor"] = report.risk.score_breakdown.cycle_factor;
    root["risk"] = risk_obj;

    // Surface
    JsonValue::ObjectType surf_obj;
    surf_obj["total_affected_tus"] = report.impact.total_affected_tus;
    surf_obj["total_affected_headers"] = report.impact.total_affected_headers;
    surf_obj["total_affected_files"] = report.impact.total_affected_files;
    surf_obj["percentage_tus_affected"] = report.impact.percentage_tus_affected;
    surf_obj["percentage_loc_affected"] = report.impact.percentage_loc_affected;
    surf_obj["max_impact_depth"] = report.impact.max_impact_depth;
    surf_obj["impact_classification"] = report.impact.impact_classification;
    root["rebuild_surface"] = surf_obj;

    // Changed files
    JsonValue::ArrayType chg_arr;
    for (const auto& chg : report.impact.changed_files) {
        JsonValue::ObjectType c_obj;
        c_obj["path"] = chg.relative_path;
        c_obj["kind"] = file_change_kind_to_string(chg.change_kind);
        chg_arr.push_back(c_obj);
    }
    root["changed_files"] = chg_arr;

    // Review hotspots
    JsonValue::ArrayType rh_arr;
    for (const auto& rh : report.risk.review_hotspots) {
        JsonValue::ObjectType r_obj;
        r_obj["path"] = rh.relative_path;
        r_obj["risk_level"] = rh.risk_level;
        r_obj["why_reason"] = rh.why_reason;
        rh_arr.push_back(r_obj);
    }
    root["review_hotspots"] = rh_arr;

    // Why risky reasons
    JsonValue::ArrayType why_arr;
    for (const auto& w : report.risk.why_risky_reasons) {
        why_arr.push_back(w);
    }
    root["why_risky"] = why_arr;

    return root;
}

std::string ImpactJsonReporter::render(const ImpactReport& report, int indent) {
    return to_json(report).serialize(indent);
}

std::string ImpactHtmlReporter::render(const ImpactReport& report, const std::string& project_name) {
    std::ostringstream html;

    html << R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>CompileForge Impact - )HTML" << project_name << R"HTML(</title>
  <style>
    :root {
      --bg: #0d1117;
      --card-bg: #161b22;
      --border: #30363d;
      --text: #c9d1d9;
      --text-muted: #8b949e;
      --accent: #58a6ff;
      --danger: #f85149;
      --warning: #d29922;
      --success: #3fb950;
    }
    body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 0; line-height: 1.5; }
    header { background: var(--card-bg); border-bottom: 1px solid var(--border); padding: 24px 40px; display: flex; justify-content: space-between; align-items: center; }
    h1 { margin: 0; font-size: 24px; color: #fff; }
    .badge { background: rgba(88, 166, 255, 0.15); color: var(--accent); padding: 4px 12px; border-radius: 12px; font-size: 13px; }
    main { max-width: 1280px; margin: 32px auto; padding: 0 24px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 20px; margin-bottom: 32px; }
    .card { background: var(--card-bg); border: 1px solid var(--border); border-radius: 8px; padding: 20px; }
    .metric-val { font-size: 32px; font-weight: 700; color: #fff; margin-top: 8px; }
    .metric-label { font-size: 14px; color: var(--text-muted); }
    .section-title { font-size: 20px; font-weight: 600; color: #fff; margin: 32px 0 16px; }
    table { width: 100%; border-collapse: collapse; background: var(--card-bg); border-radius: 8px; overflow: hidden; border: 1px solid var(--border); }
    th, td { padding: 12px 16px; text-align: left; border-bottom: 1px solid var(--border); font-size: 14px; }
    th { background: #21262d; color: #fff; }
    .tag-high { color: var(--danger); font-weight: 600; background: rgba(248, 81, 73, 0.15); padding: 2px 8px; border-radius: 4px; }
    .tag-med { color: var(--warning); font-weight: 600; background: rgba(210, 153, 34, 0.15); padding: 2px 8px; border-radius: 4px; }
    .why-card { background: var(--card-bg); border: 1px solid var(--border); border-left: 4px solid var(--danger); padding: 16px; border-radius: 8px; margin-bottom: 24px; }
    .why-bullet { margin-bottom: 6px; color: var(--text); }
  </style>
</head>
<body>
  <header>
    <div>
      <h1>CompileForge Change-Impact Report</h1>
      <span style="color: var(--text-muted); font-size: 14px;">Build Surface & Review Risk Analysis</span>
    </div>
    <div class="badge">)HTML" << project_name << R"HTML(</div>
  </header>

  <main>
    <div class="grid">
      <div class="card">
        <div class="metric-label">Change Risk Score</div>
        <div class="metric-val" style="color: )HTML" << (report.risk.score_breakdown.total_risk_score >= 70 ? "var(--danger)" : "var(--warning)") << R"HTML(;">)HTML"
        << report.risk.score_breakdown.total_risk_score << R"HTML( / 100</div>
      </div>
      <div class="card">
        <div class="metric-label">Rebuild Surface</div>
        <div class="metric-val">)HTML" << std::fixed << std::setprecision(1) << report.impact.percentage_tus_affected << R"HTML(%</div>
      </div>
      <div class="card">
        <div class="metric-label">Affected TUs / Headers</div>
        <div class="metric-val">)HTML" << report.impact.total_affected_tus << " / " << report.impact.total_affected_headers << R"HTML(</div>
      </div>
      <div class="card">
        <div class="metric-label">Review Hotspots</div>
        <div class="metric-val">)HTML" << report.risk.review_hotspots.size() << R"HTML(</div>
      </div>
    </div>

    <div class="section-title">Why This Change Is Risky</div>
    <div class="why-card">
)HTML";

    if (report.risk.why_risky_reasons.empty()) {
        html << "      <div class=\"why-bullet\">&bull; Low impact modification with minimal dependency blast radius.</div>\n";
    } else {
        for (const auto& reason : report.risk.why_risky_reasons) {
            html << "      <div class=\"why-bullet\">&bull; " << reason << "</div>\n";
        }
    }

    html << R"HTML(
    </div>

    <div class="section-title">Review Hotspots (High Attention Needed)</div>
    <table>
      <thead>
        <tr>
          <th>File Path</th>
          <th>Risk Level</th>
          <th>Why Inspect</th>
        </tr>
      </thead>
      <tbody>
)HTML";

    for (const auto& rh : report.risk.review_hotspots) {
        html << "        <tr>\n";
        html << "          <td style=\"font-family: monospace;\">" << rh.relative_path << "</td>\n";
        html << "          <td><span class=\"tag-" << (rh.risk_level == "HIGH" ? "high" : "med") << "\">" << rh.risk_level << "</span></td>\n";
        html << "          <td>" << rh.why_reason << "</td>\n";
        html << "        </tr>\n";
    }

    html << R"HTML(
      </tbody>
    </table>
  </main>
</body>
</html>
)HTML";

    return html.str();
}

} // namespace compileforge
