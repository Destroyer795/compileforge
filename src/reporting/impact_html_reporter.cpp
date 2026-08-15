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

    JsonValue::ArrayType aff_tus;
    for (const auto& aff : report.impact.affected_nodes) {
        if (aff.is_translation_unit) {
            aff_tus.push_back(aff.relative_path);
        }
    }
    surf_obj["affected_translation_units"] = aff_tus;
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

    int risk_score = report.risk.score_breakdown.total_risk_score;
    std::string risk_color = "#4d6b42"; // olive
    std::string risk_tier = "LOW RISK";
    if (risk_score >= 75) {
        risk_color = "#82261e"; // deep red-brown
        risk_tier = "CRITICAL RISK";
    } else if (risk_score >= 50) {
        risk_color = "#b84c24"; // burnt orange
        risk_tier = "HIGH RISK";
    } else if (risk_score >= 25) {
        risk_color = "#b0802c"; // mustard
        risk_tier = "MODERATE RISK";
    }

    html << R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>CompileForge Change-Impact Report &mdash; )HTML" << project_name << R"HTML(</title>
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

    /* Risk Scale Bar */
    .scale-card {
      background: var(--surface-card);
      border: 1px solid var(--border-hairline);
      padding: 24px;
      margin-bottom: 36px;
    }
    .section-headline {
      font-family: "Newsreader", "Georgia", "Baskerville", serif;
      font-size: 22px;
      font-weight: 700;
      color: var(--text-ink);
      margin-bottom: 16px;
      border-bottom: 1px solid var(--border-hairline);
      padding-bottom: 8px;
    }
    .scale-track {
      position: relative;
      height: 12px;
      background: #e5ded2;
      margin: 28px 0 12px;
      border-radius: 2px;
    }
    .scale-fill {
      height: 100%;
      background: )HTML" << risk_color << R"HTML(;
      width: )HTML" << risk_score << R"HTML(%;
      border-radius: 2px;
    }
    .scale-labels {
      display: flex;
      justify-content: space-between;
      font-family: "SF Mono", "Consolas", "Liberation Mono", monospace;
      font-size: 11px;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.08em;
    }
    .scale-indicator {
      display: flex;
      justify-content: space-between;
      align-items: baseline;
      margin-bottom: 8px;
    }
    .scale-badge {
      font-family: "SF Mono", "Consolas", monospace;
      font-size: 12px;
      font-weight: 700;
      letter-spacing: 0.1em;
      color: )HTML" << risk_color << R"HTML(;
    }

    /* Editorial List: Why This Change Is Risky */
    .editorial-list {
      margin-bottom: 36px;
    }
    .editorial-item {
      display: flex;
      gap: 20px;
      padding: 16px 0;
      border-bottom: 1px solid var(--border-hairline);
    }
    .item-num {
      font-family: "Newsreader", "Georgia", serif;
      font-size: 24px;
      font-weight: 700;
      color: var(--accent-orange);
      line-height: 1;
      width: 32px;
      flex-shrink: 0;
    }
    .item-content { flex: 1; }
    .item-text {
      font-size: 15px;
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
    .tag-high { background: #faeae6; color: var(--accent-crimson); border-color: #e5beba; }
    .tag-med { background: #faf3e6; color: var(--accent-mustard); border-color: #e8d9bc; }

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
      .masthead { flex-direction: column; align-items: flex-start; }
      .meta-col { text-align: left; }
    }
  </style>
</head>
<body>
  <div class="container">
    <header class="masthead">
      <div>
        <div class="brand-eyebrow">CompileForge // Build Intelligence</div>
        <h1 class="report-title">Change-Impact Analysis</h1>
      </div>
      <div class="meta-col">
        <div>Target: <strong>)HTML" << project_name << R"HTML(</strong></div>
        <div class="meta-tag">Static Dependency Propagation</div>
      </div>
    </header>

    <div class="metric-strip">
      <div class="metric-cell">
        <div class="metric-label">Change Risk Score</div>
        <div class="metric-val" style="color: )HTML" << risk_color << R"HTML(;">)HTML" << risk_score << R"HTML(<span class="metric-unit">/100</span></div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Rebuild Surface</div>
        <div class="metric-val">)HTML" << std::fixed << std::setprecision(1) << report.impact.percentage_tus_affected << R"HTML(<span class="metric-unit">%</span></div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Affected TUs / Headers</div>
        <div class="metric-val">)HTML" << report.impact.total_affected_tus << R"HTML(<span class="metric-unit">TUs · )HTML" << report.impact.total_affected_headers << R"HTML( H</span></div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Impact Depth</div>
        <div class="metric-val">)HTML" << report.impact.max_impact_depth << R"HTML(<span class="metric-unit">levels</span></div>
      </div>
    </div>

    <div class="scale-card">
      <div class="scale-indicator">
        <div class="section-headline" style="border-bottom:none; margin-bottom:0; padding-bottom:0;">Risk Index Calibration</div>
        <div class="scale-badge">)HTML" << risk_tier << " (" << risk_score << R"HTML(/100)</div>
      </div>
      <div class="scale-track">
        <div class="scale-fill"></div>
      </div>
      <div class="scale-labels">
        <span>Low (0–24)</span>
        <span>Moderate (25–49)</span>
        <span>High (50–74)</span>
        <span>Critical (75–100)</span>
      </div>
    </div>

    <div class="section-headline">Why This Change Is Risky</div>
    <div class="editorial-list">
)HTML";

    if (report.risk.why_risky_reasons.empty()) {
        html << "      <div class=\"editorial-item\"><div class=\"item-num\">01</div><div class=\"item-content\"><div class=\"item-text\">Low impact modification with minimal dependency blast radius.</div></div></div>\n";
    } else {
        int idx = 1;
        for (const auto& reason : report.risk.why_risky_reasons) {
            html << "      <div class=\"editorial-item\">\n";
            html << "        <div class=\"item-num\">" << (idx < 10 ? "0" : "") << idx << "</div>\n";
            html << "        <div class=\"item-content\"><div class=\"item-text\">" << reason << "</div></div>\n";
            html << "      </div>\n";
            idx++;
        }
    }

    html << R"HTML(
    </div>

    <div class="section-headline">Review Hotspots (Priority Inspection)</div>
    <table class="editorial-table">
      <thead>
        <tr>
          <th style="width: 50%;">File Path</th>
          <th style="width: 15%;">Risk Level</th>
          <th style="width: 35%;">Inspection Rationale</th>
        </tr>
      </thead>
      <tbody>
)HTML";

    if (report.risk.review_hotspots.empty()) {
        html << "        <tr><td colspan=\"3\" style=\"color: var(--text-muted); font-style: italic;\">No critical review hotspots identified.</td></tr>\n";
    } else {
        for (const auto& rh : report.risk.review_hotspots) {
            html << "        <tr>\n";
            html << "          <td class=\"path-mono\">" << rh.relative_path << "</td>\n";
            html << "          <td><span class=\"tag-editorial " << (rh.risk_level == "HIGH" ? "tag-high" : "tag-med") << "\">" << rh.risk_level << "</span></td>\n";
            html << "          <td>" << rh.why_reason << "</td>\n";
            html << "        </tr>\n";
        }
    }

    html << R"HTML(
      </tbody>
    </table>

    <div class="section-headline">Changed Files In Diff</div>
    <table class="editorial-table">
      <thead>
        <tr>
          <th style="width: 70%;">Path</th>
          <th style="width: 30%;">Change Kind</th>
        </tr>
      </thead>
      <tbody>
)HTML";

    for (const auto& chg : report.impact.changed_files) {
        html << "        <tr>\n";
        html << "          <td class=\"path-mono\">" << chg.relative_path << "</td>\n";
        html << "          <td><span class=\"tag-editorial\">" << file_change_kind_to_string(chg.change_kind) << "</span></td>\n";
        html << "        </tr>\n";
    }

    html << R"HTML(
      </tbody>
    </table>

    <div class="methodology-box">
      <div class="methodology-title">Methodology & Preprocessor Scope</div>
      This report evaluates dependency blast radius via static lexical preprocessor analysis (&gt;5.3M lines/sec). Impact represents the potential rebuild surface across downstream translation units. Historical compiler durations require <code class="path-mono">-ftime-trace</code> logs.
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
