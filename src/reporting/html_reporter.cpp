#include <compileforge/reporting/report.hpp>
#include <sstream>
#include <iomanip>

namespace compileforge {

std::string HtmlReporter::render(const AnalysisReport& report, const std::string& project_name) {
    std::ostringstream html;

    html << R"HTML(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>CompileForge - )HTML" << project_name << R"HTML(</title>
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
      --purple: #bc8cff;
    }
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      background-color: var(--bg);
      color: var(--text);
      margin: 0;
      padding: 0;
      line-height: 1.5;
    }
    header {
      background-color: var(--card-bg);
      border-bottom: 1px solid var(--border);
      padding: 24px 40px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    h1 { margin: 0; font-size: 24px; color: #fff; font-weight: 600; }
    .badge {
      background: rgba(88, 166, 255, 0.15);
      color: var(--accent);
      padding: 4px 12px;
      border-radius: 12px;
      font-size: 13px;
      font-weight: 500;
    }
    main { max-width: 1280px; margin: 32px auto; padding: 0 24px; }
    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); gap: 20px; margin-bottom: 32px; }
    .card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 20px;
    }
    .metric-val { font-size: 32px; font-weight: 700; color: #fff; margin-top: 8px; }
    .metric-label { font-size: 14px; color: var(--text-muted); font-weight: 500; }
    
    .section-title { font-size: 20px; font-weight: 600; color: #fff; margin: 32px 0 16px; display: flex; align-items: center; justify-content: space-between; }
    
    .search-input {
      background: var(--bg);
      border: 1px solid var(--border);
      color: #fff;
      padding: 8px 14px;
      border-radius: 6px;
      font-size: 14px;
      width: 260px;
    }

    table { width: 100%; border-collapse: collapse; background: var(--card-bg); border-radius: 8px; overflow: hidden; border: 1px solid var(--border); }
    th, td { padding: 12px 16px; text-align: left; border-bottom: 1px solid var(--border); font-size: 14px; }
    th { background: #21262d; color: #fff; font-weight: 600; }
    tr:hover { background: rgba(255, 255, 255, 0.02); }

    .tag-high { color: var(--danger); font-weight: 600; background: rgba(248, 81, 73, 0.15); padding: 2px 8px; border-radius: 4px; }
    .tag-med { color: var(--warning); font-weight: 600; background: rgba(210, 153, 34, 0.15); padding: 2px 8px; border-radius: 4px; }
    .tag-low { color: var(--accent); font-weight: 600; background: rgba(88, 166, 255, 0.15); padding: 2px 8px; border-radius: 4px; }
    
    .rec-card {
      background: var(--card-bg);
      border: 1px solid var(--border);
      border-radius: 8px;
      padding: 16px;
      margin-bottom: 12px;
      border-left: 4px solid var(--accent);
    }
    .rec-card.high { border-left-color: var(--danger); }
    .rec-card.med { border-left-color: var(--warning); }
    .rec-title { font-size: 16px; font-weight: 600; color: #fff; margin-bottom: 4px; }
    .rec-desc { color: var(--text-muted); font-size: 14px; margin-bottom: 8px; }
    .rec-action { background: rgba(255, 255, 255, 0.04); padding: 8px 12px; border-radius: 4px; font-size: 13px; font-family: monospace; color: var(--text); }
  </style>
</head>
<body>
  <header>
    <div>
      <h1>CompileForge Report</h1>
      <span style="color: var(--text-muted); font-size: 14px;">Build Intelligence & Architecture Analysis</span>
    </div>
    <div class="badge">)HTML" << project_name << R"HTML(</div>
  </header>

  <main>
    <div class="grid">
      <div class="card">
        <div class="metric-label">Total Files</div>
        <div class="metric-val">)HTML" << report.summary.total_files << R"HTML(</div>
      </div>
      <div class="card">
        <div class="metric-label">Headers / TUs</div>
        <div class="metric-val">)HTML" << report.summary.total_headers << " / " << report.summary.total_translation_units << R"HTML(</div>
      </div>
      <div class="card">
        <div class="metric-label">Total SLOC</div>
        <div class="metric-val">)HTML" << report.summary.total_sloc << R"HTML(</div>
      </div>
      <div class="card">
        <div class="metric-label">Circular Dependencies</div>
        <div class="metric-val" style="color: )HTML" << (report.summary.circular_dependency_count > 0 ? "var(--danger)" : "var(--success)") << R"HTML(;">)HTML"
        << report.summary.circular_dependency_count << R"HTML(</div>
      </div>
    </div>

    <div class="section-title">
      <span>Actionable Recommendations</span>
    </div>
)HTML";

    if (report.recommendations.empty()) {
        html << "<div class=\"card\" style=\"color: var(--success);\">No critical recommendations. Architecture is clean!</div>";
    } else {
        for (const auto& rec : report.recommendations) {
            std::string sev_class = (rec.severity == Severity::High) ? "high" : (rec.severity == Severity::Medium ? "med" : "low");
            html << "<div class=\"rec-card " << sev_class << "\">\n";
            html << "  <div class=\"rec-title\"><span class=\"tag-" << sev_class << "\">" << severity_to_string(rec.severity) << "</span> " << rec.title << "</div>\n";
            html << "  <div class=\"rec-desc\">" << rec.description << "</div>\n";
            html << "  <div class=\"rec-action\">&rarr; " << rec.actionable_step << "</div>\n";
            html << "</div>\n";
        }
    }

    html << R"HTML(
    <div class="section-title">
      <span>Build & Dependency Hotspots</span>
      <input type="text" class="search-input" id="search" placeholder="Filter files..." onkeyup="filterTable()">
    </div>
    <table id="hotspotsTable">
      <thead>
        <tr>
          <th>File Path</th>
          <th>Type</th>
          <th>SLOC</th>
          <th>Transitive Dependents</th>
          <th>Est. Compile Time</th>
          <th>Hotspot Score</th>
          <th>Score Breakdown</th>
        </tr>
      </thead>
      <tbody>
)HTML";

    for (size_t i = 0; i < report.top_hotspots.size(); ++i) {
        const auto& node = report.top_hotspots[i];
        html << "        <tr>\n";
        html << "          <td style=\"font-family: monospace; font-weight: 500;\">" << node.relative_path << "</td>\n";
        html << "          <td>" << ((node.kind == FileKind::Header) ? "Header" : "TranslationUnit") << "</td>\n";
        html << "          <td>" << node.metrics.sloc << "</td>\n";
        html << "          <td>" << node.fan_stats.fan_in_transitive << "</td>\n";
        html << "          <td>" << std::fixed << std::setprecision(2) << node.build_time.compilation_seconds << "s</td>\n";
        html << "          <td><span class=\"badge\">" << std::setprecision(1) << node.hotspot.total_score << "</span></td>\n";
        html << "          <td style=\"font-size: 12px; color: var(--text-muted);\">" << node.hotspot.score_breakdown << "</td>\n";
        html << "        </tr>\n";
    }

    html << R"HTML(
      </tbody>
    </table>
  </main>

  <script>
    function filterTable() {
      var input = document.getElementById('search');
      var filter = input.value.toLowerCase();
      var table = document.getElementById('hotspotsTable');
      var tr = table.getElementsByTagName('tr');

      for (var i = 1; i < tr.length; i++) {
        var td = tr[i].getElementsByTagName('td')[0];
        if (td) {
          var textValue = td.textContent || td.innerText;
          if (textValue.toLowerCase().indexOf(filter) > -1) {
            tr[i].style.display = '';
          } else {
            tr[i].style.display = 'none';
          }
        }
      }
    }
  </script>
</body>
</html>
)HTML";

    return html.str();
}

} // namespace compileforge
