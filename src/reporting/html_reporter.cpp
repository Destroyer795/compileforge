#include "../../include/compileforge/reporting/report.hpp"
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
  <title>CompileForge Build Intelligence &mdash; )HTML" << project_name << R"HTML(</title>
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

    .section-headline {
      font-family: "Newsreader", "Georgia", "Baskerville", serif;
      font-size: 22px;
      font-weight: 700;
      color: var(--text-ink);
      margin-bottom: 16px;
      border-bottom: 1px solid var(--border-hairline);
      padding-bottom: 8px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    /* Editorial Recommendations */
    .rec-item {
      background: var(--surface-card);
      border: 1px solid var(--border-hairline);
      padding: 18px 20px;
      margin-bottom: 14px;
    }
    .rec-item.high { border-left: 4px solid var(--accent-crimson); }
    .rec-item.med { border-left: 4px solid var(--accent-mustard); }
    .rec-item.low { border-left: 4px solid var(--accent-teal); }
    
    .rec-header {
      display: flex;
      align-items: center;
      gap: 10px;
      margin-bottom: 6px;
    }
    .rec-title {
      font-size: 15px;
      font-weight: 700;
      color: var(--text-ink);
    }
    .rec-desc {
      font-size: 14px;
      color: var(--text-muted);
      margin-bottom: 10px;
    }
    .rec-action {
      font-family: "SF Mono", "Consolas", monospace;
      font-size: 12px;
      background: #eee8dc;
      padding: 6px 10px;
      border-radius: 2px;
      color: var(--text-ink);
      display: inline-block;
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
      font-size: 13px;
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
    .tag-low { background: #e8eff0; color: var(--accent-teal); border-color: #bed0d2; }

    .search-input {
      font-family: "SF Mono", "Consolas", monospace;
      font-size: 12px;
      padding: 6px 10px;
      border: 1px solid var(--border-hairline);
      background: var(--surface-card);
      color: var(--text-ink);
      outline: none;
      width: 200px;
    }

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
        <div class="brand-eyebrow">CompileForge Report // Build Intelligence</div>
        <h1 class="report-title">Build Architecture & Health Analysis</h1>
      </div>
      <div class="meta-col">
        <div>Project: <strong>)HTML" << project_name << R"HTML(</strong></div>
        <div class="meta-tag">Whole-Project Graph Analysis</div>
      </div>
    </header>

    <div class="metric-strip">
      <div class="metric-cell">
        <div class="metric-label">Total Files</div>
        <div class="metric-val">)HTML" << report.summary.total_files << R"HTML(</div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Headers / TUs</div>
        <div class="metric-val">)HTML" << report.summary.total_headers << R"HTML(<span class="metric-unit">H · )HTML" << report.summary.total_translation_units << R"HTML( TU</span></div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Total SLOC</div>
        <div class="metric-val">)HTML" << report.summary.total_sloc << R"HTML(</div>
      </div>
      <div class="metric-cell">
        <div class="metric-label">Circular Includes</div>
        <div class="metric-val" style="color: )HTML" << (report.summary.circular_dependency_count > 0 ? "var(--accent-crimson)" : "var(--accent-olive)") << R"HTML(;">)HTML"
        << report.summary.circular_dependency_count << R"HTML(</div>
      </div>
    </div>

    <div class="section-headline">Actionable Architecture Recommendations</div>
)HTML";

    if (report.recommendations.empty()) {
        html << "    <div class=\"rec-item low\"><div class=\"rec-title\" style=\"color: var(--accent-olive);\">Architecture is Clean</div><div class=\"rec-desc\">No critical circular inclusion loops or oversized translation unit bottlenecks identified.</div></div>\n";
    } else {
        for (const auto& rec : report.recommendations) {
            std::string sev_class = (rec.severity == Severity::High) ? "high" : (rec.severity == Severity::Medium ? "med" : "low");
            std::string sev_label = (rec.severity == Severity::High) ? "HIGH" : (rec.severity == Severity::Medium ? "MEDIUM" : "LOW");
            html << "    <div class=\"rec-item " << sev_class << "\">\n";
            html << "      <div class=\"rec-header\">\n";
            html << "        <span class=\"tag-editorial tag-" << sev_class << "\">" << sev_label << "</span>\n";
            html << "        <span class=\"rec-title\">" << rec.title << "</span>\n";
            html << "      </div>\n";
            html << "      <div class=\"rec-desc\">" << rec.description << "</div>\n";
            html << "      <div class=\"rec-action\">&rarr; " << rec.actionable_step << "</div>\n";
            html << "    </div>\n";
        }
    }

    html << R"HTML(
    <div class="section-headline" style="margin-top: 36px;">
      <span>Build & Dependency Hotspots</span>
      <input type="text" class="search-input" id="search" placeholder="Filter files..." onkeyup="filterTable()">
    </div>

    <table class="editorial-table" id="hotspotsTable">
      <thead>
        <tr>
          <th style="width: 35%;">File Path</th>
          <th style="width: 15%;">Kind</th>
          <th style="width: 10%;">SLOC</th>
          <th style="width: 15%;">Transitive Fan-In</th>
          <th style="width: 15%;">Hotspot Score</th>
        </tr>
      </thead>
      <tbody>
)HTML";

    for (size_t i = 0; i < report.top_hotspots.size(); ++i) {
        const auto& node = report.top_hotspots[i];
        html << "        <tr>\n";
        html << "          <td class=\"path-mono\">" << node.relative_path << "</td>\n";
        html << "          <td>" << ((node.kind == FileKind::Header) ? "Header" : "TranslationUnit") << "</td>\n";
        html << "          <td>" << node.metrics.sloc << "</td>\n";
        html << "          <td>" << node.fan_stats.fan_in_transitive << "</td>\n";
        html << "          <td><span class=\"tag-editorial tag-high\">" << std::fixed << std::setprecision(1) << node.hotspot.total_score << "</span></td>\n";
        html << "        </tr>\n";
    }

    html << R"HTML(
      </tbody>
    </table>

    <div class="methodology-box">
      <div class="methodology-title">Graph Analysis Methodology</div>
      CompileForge builds a directed graph of all C++ headers and translation units. Transitive fan-in reflects the number of files directly or indirectly affected by changes to that file. Circular inclusion loops are computed via Tarjan's Strongly Connected Components algorithm.
    </div>

    <footer class="colophon">
      <div>CompileForge &middot; Native C++20 Build Intelligence</div>
      <div>Zero Third-Party Runtime Dependencies &middot; MIT License</div>
    </footer>
  </div>

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
</html>)HTML";

    return html.str();
}

} // namespace compileforge
