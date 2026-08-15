#include "../test_framework.hpp"
#include "../../include/compileforge/core/utils.hpp"
#include "../../include/compileforge/project/scanner.hpp"
#include "../../include/compileforge/parser/compilation_database.hpp"
#include "../../include/compileforge/parser/include_parser.hpp"
#include "../../include/compileforge/graph/dependency_graph.hpp"
#include "../../include/compileforge/graph/cycle_detector.hpp"
#include "../../include/compileforge/metrics/source_metrics.hpp"
#include "../../include/compileforge/analysis/hotspot_scorer.hpp"
#include "../../include/compileforge/recommendations/recommendation_engine.hpp"
#include "../../include/compileforge/reporting/report.hpp"
#include <filesystem>

using namespace compileforge;

TEST_CASE(test_e2e_monolith_app_analysis) {
    std::string proj_root = "examples/monolith_app";
    if (!std::filesystem::exists(proj_root)) return;

    ScanOptions scan_opts;
    scan_opts.root_path = proj_root;
    ProjectScanner scanner(scan_opts);

    auto scan_res = scanner.scan();
    ASSERT_TRUE(scan_res.has_value());
    auto files = scan_res.value();
    ASSERT_TRUE(files.size() >= 5);

    CompilationDatabase comp_db;
    std::filesystem::path db_file = std::filesystem::path(proj_root) / "compile_commands.json";
    if (std::filesystem::exists(db_file)) {
        auto db_res = CompilationDatabase::load_file(db_file.string());
        if (db_res.has_value()) comp_db = db_res.value();
    }

    DependencyGraph graph;
    for (auto& file : files) {
        auto parse_res = IncludeParser::parse_file(file.canonical_path);
        if (parse_res.has_value()) {
            file.metrics = parse_res.value().metrics;
            file.includes = parse_res.value().includes;
        }

        const auto* comp_entry = comp_db.find_entry(file.canonical_path);
        std::vector<std::string> inc_dirs;
        if (comp_entry) inc_dirs = comp_entry->include_dirs;

        for (auto& inc : file.includes) {
            inc.resolved_path = IncludeParser::resolve_include_path(inc, file.canonical_path, inc_dirs, proj_root);
            if (!inc.resolved_path.empty()) {
                std::string rel_inc = utils::to_relative_path(inc.resolved_path, proj_root);
                graph.add_edge(file.relative_path, rel_inc);
            }
        }
        graph.add_node(file);
    }

    graph.compute_fan_stats();
    HotspotScorer::compute_hotspots(graph);

    auto cycles = CycleDetector::detect_cycles(graph);
    ASSERT_EQ(cycles.size(), 0);

    ProjectSummary summary = SourceMetrics::compute_summary(graph, cycles.size());
    ASSERT_TRUE(summary.total_files >= 5);

    AnalysisReport report;
    report.summary = summary;
    report.cycles = cycles;
    report.top_hotspots = HotspotScorer::get_top_hotspots(graph, 5);

    std::string json_str = JsonReporter::render(report, -1);
    ASSERT_TRUE(!json_str.empty());

    std::string html_str = HtmlReporter::render(report, "Monolith App");
    ASSERT_TRUE(html_str.find("CompileForge Report") != std::string::npos);
}

TEST_CASE(test_e2e_circular_includes_detection) {
    std::string proj_root = "examples/circular_includes";
    if (!std::filesystem::exists(proj_root)) return;

    ScanOptions scan_opts;
    scan_opts.root_path = proj_root;
    ProjectScanner scanner(scan_opts);

    auto scan_res = scanner.scan();
    ASSERT_TRUE(scan_res.has_value());
    auto files = scan_res.value();

    DependencyGraph graph;
    for (auto& file : files) {
        auto parse_res = IncludeParser::parse_file(file.canonical_path);
        if (parse_res.has_value()) {
            file.includes = parse_res.value().includes;
        }
        for (auto& inc : file.includes) {
            inc.resolved_path = IncludeParser::resolve_include_path(inc, file.canonical_path, {}, proj_root);
            if (!inc.resolved_path.empty()) {
                std::string rel_inc = utils::to_relative_path(inc.resolved_path, proj_root);
                graph.add_edge(file.relative_path, rel_inc);
            }
        }
        graph.add_node(file);
    }

    auto cycles = CycleDetector::detect_cycles(graph);
    ASSERT_TRUE(cycles.size() >= 1);
}
