#include <compileforge/core/result.hpp>
#include <compileforge/core/utils.hpp>
#include <compileforge/project/scanner.hpp>
#include <compileforge/parser/compilation_database.hpp>
#include <compileforge/parser/include_parser.hpp>
#include <compileforge/graph/dependency_graph.hpp>
#include <compileforge/graph/cycle_detector.hpp>
#include <compileforge/metrics/source_metrics.hpp>
#include <compileforge/build/build_trace.hpp>
#include <compileforge/git/git_analyzer.hpp>
#include <compileforge/analysis/hotspot_scorer.hpp>
#include <compileforge/analysis/regression_detector.hpp>
#include <compileforge/analysis/build_config_health.hpp>
#include <compileforge/analysis/include_optimizer.hpp>
#include <compileforge/analysis/tu_cost_analyzer.hpp>
#include <compileforge/analysis/build_health_score.hpp>
#include <compileforge/config/config.hpp>
#include <compileforge/recommendations/recommendation_engine.hpp>
#include <compileforge/config/config.hpp>
#include <compileforge/cache/analysis_cache.hpp>
#include <compileforge/reporting/report.hpp>
#include <iostream>
#include <fstream>

using namespace compileforge;

void print_usage() {
    std::cout << R"(
CompileForge - C++ Build Intelligence & Optimization Toolkit

USAGE:
  compileforge <subcommand> [options]

SUBCOMMANDS:
  analyze <project_path>    Analyze compilation database and include graph
  diff <old.json> <new.json> Compare two analysis reports for regressions
  init [project_path]       Initialize a .compileforge.json configuration file
  --help, -h                Show this help message
  --version, -v             Show version information

ANALYZE OPTIONS:
  --compilation-database, -db <path> Path to compile_commands.json
  --format <terminal|json|html>      Output format (default: terminal)
  --output, -o <file>                Save report to file
  --fail-on-cycle                    Exit with non-zero code if cycles detected
  --fail-on-hotspot                  Exit with non-zero code if hotspots score > 80
  --quiet, -q                        Suppress non-essential terminal output
  --no-color                         Disable ANSI terminal colors
)";
}

int handle_analyze(int argc, char* argv[]) {
    std::string project_path = ".";
    std::string comp_db_path;
    std::string format = "terminal";
    std::string output_file;
    bool fail_on_cycle = false;
    bool fail_on_hotspot = false;
    bool quiet = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--compilation-database" || arg == "-db") {
            if (i + 1 < argc) comp_db_path = argv[++i];
        } else if (arg == "--format") {
            if (i + 1 < argc) format = argv[++i];
        } else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) output_file = argv[++i];
        } else if (arg == "--fail-on-cycle") {
            fail_on_cycle = true;
        } else if (arg == "--fail-on-hotspot") {
            fail_on_hotspot = true;
        } else if (arg == "--quiet" || arg == "-q") {
            quiet = true;
        } else if (arg == "--no-color") {
            // Handled via std::getenv or isatty
        } else if (arg[0] != '-') {
            project_path = arg;
        }
    }

    project_path = utils::normalize_path(project_path);
    std::string cfg_filepath = (std::filesystem::path(project_path) / ".compileforge.json").string();
    auto cfg_res = Config::load_file(cfg_filepath);
    Config config = cfg_res.is_ok() ? cfg_res.value() : Config::default_config();

    ScanOptions scan_opts;
    scan_opts.root_path = project_path;
    scan_opts.ignore_patterns = config.ignore_patterns;

    ProjectScanner scanner(scan_opts);
    auto scan_res = scanner.scan();
    if (scan_res.is_error()) {
        std::cerr << "Error scanning project: " << scan_res.error().message << "\n";
        return 1;
    }

    auto files = scan_res.value();
    DependencyGraph graph;
    std::vector<std::string> search_dirs;

    if (comp_db_path.empty()) {
        comp_db_path = (std::filesystem::path(project_path) / "compile_commands.json").string();
    }

    CompilationDatabase comp_db;
    if (std::filesystem::exists(comp_db_path)) {
        auto db_res = CompilationDatabase::load_file(comp_db_path);
        if (db_res.is_ok()) {
            comp_db = db_res.value();
            for (const auto& entry : comp_db.entries()) {
                for (const auto& inc : entry.include_dirs) {
                    search_dirs.push_back(inc);
                }
            }
        }
    }

    // Build dependency graph
    for (auto& file : files) {
        graph.add_node(file);
        auto parse_res = IncludeParser::parse_file(file.canonical_path);
        if (parse_res.is_ok()) {
            file.metrics = parse_res.value().metrics;
            file.includes = parse_res.value().includes;

            for (const auto& inc : file.includes) {
                std::string resolved = IncludeParser::resolve_include_path(inc, file.canonical_path, search_dirs, project_path);
                if (!resolved.empty()) {
                    graph.add_edge(file.relative_path, utils::to_relative_path(resolved, project_path));
                }
            }
        }
    }

    HotspotScorer::compute_hotspots(graph);
    auto cycles = CycleDetector::detect_cycles(graph);
    auto build_config_findings = BuildConfigHealthAnalyzer::analyze(comp_db);
    auto tu_cost_profiles = TUCostAnalyzer::analyze_all_tus(graph);
    auto health_score = BuildHealthScorer::compute_score(graph, cycles.size(), build_config_findings);
    auto recs = RecommendationEngine::generate_recommendations(graph, cycles, config);

    AnalysisReport report;
    report.summary.total_files = files.size();
    report.summary.total_headers = 0;
    report.summary.total_translation_units = 0;
    report.summary.total_loc = 0;
    report.summary.total_sloc = 0;
    report.summary.circular_dependency_count = cycles.size();

    for (const auto& f : files) {
        if (f.kind == FileKind::Header) report.summary.total_headers++;
        else if (f.kind == FileKind::TranslationUnit) report.summary.total_translation_units++;
        report.summary.total_loc += f.metrics.total_lines;
        report.summary.total_sloc += f.metrics.sloc;
    }

    report.health_score = health_score;
    report.files = files;
    report.top_hotspots = HotspotScorer::get_top_hotspots(graph, 10);
    report.cycles = cycles;
    report.recommendations = recs;
    report.build_config_findings = build_config_findings;
    report.tu_cost_profiles = tu_cost_profiles;

    if (format == "json") {
        std::string json_str = JsonReporter::render(report);
        if (!output_file.empty()) {
            std::ofstream ofs(output_file);
            ofs << json_str;
            if (!quiet) std::cout << "JSON report saved to: " << output_file << "\n";
        } else {
            std::cout << json_str << "\n";
        }
    } else if (format == "html") {
        std::string html_str = HtmlReporter::render(report, project_path);
        if (!output_file.empty()) {
            std::ofstream ofs(output_file);
            ofs << html_str;
            if (!quiet) std::cout << "HTML report saved to: " << output_file << "\n";
        } else {
            std::cout << html_str << "\n";
        }
    } else {
        if (!quiet) {
            TerminalReporter::print(report);
        }
    }

    if (fail_on_cycle && !cycles.empty()) {
        std::cerr << "CI Failure: " << cycles.size() << " circular dependency loop(s) detected.\n";
        return 1;
    }
    if (fail_on_hotspot && !report.top_hotspots.empty() && report.top_hotspots.front().hotspot.total_score > 80.0) {
        std::cerr << "CI Failure: Hotspot score exceeds threshold 80.0.\n";
        return 1;
    }

    return 0;
}

int handle_diff(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: compileforge diff <baseline.json> <target.json>\n";
        return 1;
    }
    std::string base_path = argv[2];
    std::string target_path = argv[3];

    std::ifstream f1(base_path), f2(target_path);
    if (!f1 || !f2) {
        std::cerr << "Error: Could not open baseline or target report JSON files.\n";
        return 1;
    }
    std::string c1((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    std::string c2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());

    auto j1 = JsonValue::parse(c1);
    auto j2 = JsonValue::parse(c2);
    if (j1.is_error() || j2.is_error()) {
        std::cerr << "Error parsing report JSON for diff.\n";
        return 1;
    }

    auto diff_res = RegressionDetector::compare_reports(j1.value(), j2.value());
    if (diff_res.is_regression) {
        std::cerr << "REGRESSION DETECTED!\n";
        for (const auto& msg : diff_res.regression_messages) {
            std::cerr << "  " << msg << "\n";
        }
        return 1;
    }

    std::cout << "SUCCESS: No build regressions detected between baseline and current runs.\n";
    return 0;
}

int handle_init(int argc, char* argv[]) {
    std::string proj = ".";
    if (argc >= 3) proj = argv[2];
    std::string cfg_path = (std::filesystem::path(proj) / ".compileforge.json").string();
    if (std::filesystem::exists(cfg_path)) {
        std::cout << "Configuration file already exists at: " << cfg_path << "\n";
        return 0;
    }
    Config opts = Config::default_config();
    if (Config::save_config(cfg_path, opts)) {
        std::cout << "Initialized CompileForge configuration at: " << cfg_path << "\n";
        return 0;
    }
    std::cerr << "Failed to write configuration file.\n";
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 0;
    }

    std::string command = argv[1];
    if (command == "--help" || command == "-h") {
        print_usage();
        return 0;
    } else if (command == "--version" || command == "-v") {
        std::cout << "CompileForge v1.0.0 (C++20 Build Intelligence Toolkit)\n";
        return 0;
    } else if (command == "analyze") {
        return handle_analyze(argc, argv);
    } else if (command == "diff") {
        return handle_diff(argc, argv);
    } else if (command == "init") {
        return handle_init(argc, argv);
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        print_usage();
        return 1;
    }
}
