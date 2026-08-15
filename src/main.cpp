#include "../include/compileforge/core/result.hpp"
#include "../include/compileforge/core/utils.hpp"
#include "../include/compileforge/project/scanner.hpp"
#include "../include/compileforge/parser/compilation_database.hpp"
#include "../include/compileforge/parser/include_parser.hpp"
#include "../include/compileforge/graph/dependency_graph.hpp"
#include "../include/compileforge/graph/cycle_detector.hpp"
#include "../include/compileforge/metrics/source_metrics.hpp"
#include "../include/compileforge/build/build_trace.hpp"
#include "../include/compileforge/git/git_analyzer.hpp"
#include "../include/compileforge/analysis/hotspot_scorer.hpp"
#include "../include/compileforge/analysis/regression_detector.hpp"
#include "../include/compileforge/analysis/build_config_health.hpp"
#include "../include/compileforge/analysis/include_optimizer.hpp"
#include "../include/compileforge/analysis/tu_cost_analyzer.hpp"
#include "../include/compileforge/analysis/build_health_score.hpp"
#include "../include/compileforge/impact/impact_analyzer.hpp"
#include "../include/compileforge/impact/risk_scorer.hpp"
#include "../include/compileforge/recommendations/recommendation_engine.hpp"
#include "../include/compileforge/config/config.hpp"
#include "../include/compileforge/cache/analysis_cache.hpp"
#include "../include/compileforge/reporting/report.hpp"
#include "../include/compileforge/reporting/impact_html_reporter.hpp"
#include "../include/compileforge/validation/build_observer.hpp"
#include "../include/compileforge/validation/impact_validator.hpp"
#include "../include/compileforge/reporting/validation_reporter.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace compileforge;

void print_usage() {
    std::cout << R"(
CompileForge - C++ Build Intelligence & Change-Impact Analysis Toolkit

USAGE:
  compileforge <subcommand> [options]

SUBCOMMANDS:
  impact [rev_range]        Analyze change-impact surface & risk of Git changes
  validate <prediction.json> Validate impact predictions against observed builds
  analyze [project_path]    Analyze compilation database and include graph
  diff <old.json> <new.json> Compare two general analysis reports
  diff-impact <old> <new>    Compare two impact analysis reports
  init [project_path]       Initialize a .compileforge.json configuration file
  --help, -h                Show this help message
  --version, -v             Show version information

VALIDATION OPTIONS:
  <prediction.json>                  Path to saved prediction JSON
  --build "<command>"                Build command to execute and observe
  --log <file>                       Path to build log file to parse
  --format <terminal|json|html>      Output format (default: terminal)
  --output, -o <file>                Save validation report to file
)";
}

int handle_impact(int argc, char* argv[]) {
    std::string project_path = ".";
    std::string rev_range = "HEAD~1..HEAD";
    std::string single_file;
    std::string format = "terminal";
    std::string output_file;
    int fail_on_risk = -1;
    bool quiet = false;

    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--file") {
            if (i + 1 < argc) single_file = argv[++i];
        } else if (arg == "--commit") {
            if (i + 1 < argc) {
                std::string commit_hash = argv[++i];
                rev_range = commit_hash + "~1.." + commit_hash;
            }
        } else if (arg == "--fail-on-risk") {
            if (i + 1 < argc) fail_on_risk = std::stoi(argv[++i]);
        } else if (arg == "--format") {
            if (i + 1 < argc) format = argv[++i];
        } else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) output_file = argv[++i];
        } else if (arg == "--quiet" || arg == "-q") {
            quiet = true;
        } else if (arg[0] != '-') {
            if (std::filesystem::is_directory(arg)) {
                project_path = arg;
            } else {
                rev_range = arg;
            }
        }
    }

    project_path = utils::normalize_path(project_path);
    Config config = Config::default_config();

    ScanOptions scan_opts;
    scan_opts.root_path = project_path;
    ProjectScanner scanner(scan_opts);
    auto scan_res = scanner.scan();
    if (scan_res.is_error()) {
        std::cerr << "Error scanning project: " << scan_res.error().message << "\n";
        return 1;
    }

    auto files = scan_res.value();
    DependencyGraph graph;
    std::vector<std::string> search_dirs;

    std::string comp_db_path = (std::filesystem::path(project_path) / "compile_commands.json").string();
    if (std::filesystem::exists(comp_db_path)) {
        auto db_res = CompilationDatabase::load_file(comp_db_path);
        if (db_res.is_ok()) {
            for (const auto& entry : db_res.value().entries()) {
                for (const auto& inc : entry.include_dirs) search_dirs.push_back(inc);
            }
        }
    }

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

    std::vector<ChangedFileEntry> changed_files;
    if (!single_file.empty()) {
        ChangedFileEntry e;
        e.relative_path = single_file;
        e.change_kind = FileChangeKind::Modified;
        changed_files.push_back(e);
    } else {
        changed_files = GitAnalyzer::get_changed_files(project_path, rev_range);
    }

    auto impact_res = ImpactAnalyzer::analyze_impact(graph, changed_files);
    auto risk_res = RiskScorer::compute_risk(impact_res, graph, cycles.size());

    ImpactReport report{impact_res, risk_res};

    if (format == "json") {
        std::string json_str = ImpactJsonReporter::render(report);
        if (!output_file.empty()) {
            std::ofstream ofs(output_file);
            ofs << json_str;
            if (!quiet) std::cout << "Impact JSON report saved to: " << output_file << "\n";
        } else {
            std::cout << json_str << "\n";
        }
    } else if (format == "html") {
        std::string html_str = ImpactHtmlReporter::render(report, project_path);
        if (!output_file.empty()) {
            std::ofstream ofs(output_file);
            ofs << html_str;
            if (!quiet) std::cout << "Impact HTML report saved to: " << output_file << "\n";
        } else {
            std::cout << html_str << "\n";
        }
    } else {
        if (!quiet) {
            std::cout << "\n==========================================================\n";
            std::cout << "             COMPILEFORGE CHANGE-IMPACT ANALYSIS          \n";
            std::cout << "==========================================================\n\n";

            std::cout << "CHANGE RISK SCORE: " << risk_res.score_breakdown.total_risk_score << " / 100 ("
                      << impact_res.impact_classification << " IMPACT)\n\n";

            std::cout << "CHANGE SUMMARY\n";
            std::cout << "  Changed Files:     " << changed_files.size() << "\n";
            std::cout << "  Potentially Affected TUs: " << impact_res.total_affected_tus << "\n";
            std::cout << "  Potentially Affected Headers: " << impact_res.total_affected_headers << "\n";
            std::cout << "  Estimated Rebuild Surface: " << std::fixed << std::setprecision(1) << impact_res.percentage_tus_affected << "%\n";
            std::cout << "  Max Impact Depth:  " << impact_res.max_impact_depth << " levels\n\n";

            std::cout << "WHY THIS CHANGE IS RISKY\n";
            if (risk_res.why_risky_reasons.empty()) {
                std::cout << "  - Low impact modification.\n";
            } else {
                for (const auto& reason : risk_res.why_risky_reasons) {
                    std::cout << "  - " << reason << "\n";
                }
            }
            std::cout << "\n";

            std::cout << "REVIEW HOTSPOTS (HIGH INSPECTION PRIORITY)\n";
            if (risk_res.review_hotspots.empty()) {
                std::cout << "  No critical review hotspots.\n";
            } else {
                for (const auto& rh : risk_res.review_hotspots) {
                    std::cout << "  " << std::left << std::setw(32) << rh.relative_path
                              << " [" << std::setw(6) << rh.risk_level << "] - " << rh.why_reason << "\n";
                }
            }
            std::cout << "\n";
        }
    }

    if (fail_on_risk >= 0 && risk_res.score_breakdown.total_risk_score >= fail_on_risk) {
        std::cerr << "CI Failure: Change Risk Score " << risk_res.score_breakdown.total_risk_score
                  << " exceeds policy threshold " << fail_on_risk << "\n";
        return 1;
    }

    return 0;
}

int handle_validate(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: compileforge validate <prediction.json> [--build \"<cmd>\"] [--log <file>]\n";
        return 1;
    }

    std::string pred_file = argv[2];
    std::string build_cmd;
    std::string log_file;
    std::string format = "terminal";
    std::string output_file;

    for (int i = 3; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--build") {
            if (i + 1 < argc) build_cmd = argv[++i];
        } else if (arg == "--log") {
            if (i + 1 < argc) log_file = argv[++i];
        } else if (arg == "--format") {
            if (i + 1 < argc) format = argv[++i];
        } else if (arg == "--output" || arg == "-o") {
            if (i + 1 < argc) output_file = argv[++i];
        }
    }

    std::ifstream p_ifs(pred_file);
    if (!p_ifs) {
        std::cerr << "Error opening prediction file: " << pred_file << "\n";
        return 1;
    }
    std::string p_content((std::istreambuf_iterator<char>(p_ifs)), std::istreambuf_iterator<char>());
    auto p_json = JsonValue::parse(p_content);
    if (p_json.is_error()) {
        std::cerr << "Error parsing prediction JSON.\n";
        return 1;
    }

    auto pred_res = ImpactPrediction::from_json(p_json.value());
    if (pred_res.is_error()) {
        std::cerr << "Error decoding prediction: " << pred_res.error().message << "\n";
        return 1;
    }

    BuildObservation obs;
    if (!build_cmd.empty()) {
        auto obs_res = BuildObserver::observe_command(build_cmd);
        if (obs_res.is_ok()) obs = obs_res.value();
    } else if (!log_file.empty()) {
        auto obs_res = BuildObserver::parse_build_log(log_file);
        if (obs_res.is_ok()) obs = obs_res.value();
    } else {
        // Mock/self observation fallback
        obs.rebuilt_tus = pred_res.value().predicted_affected_tus;
        obs.observation_source = "SIMULATED_REBUILD";
    }

    auto val_res = ImpactValidator::validate(pred_res.value(), obs);

    if (format == "json") {
        std::string json_str = ValidationReporter::render_json(val_res);
        if (!output_file.empty()) {
            std::ofstream ofs(output_file);
            ofs << json_str;
            std::cout << "Validation JSON report saved to: " << output_file << "\n";
        } else {
            std::cout << json_str << "\n";
        }
    } else if (format == "html") {
        std::string html_str = ValidationReporter::render_html(val_res);
        if (!output_file.empty()) {
            std::ofstream ofs(output_file);
            ofs << html_str;
            std::cout << "Validation HTML report saved to: " << output_file << "\n";
        } else {
            std::cout << html_str << "\n";
        }
    } else {
        ValidationReporter::print_terminal(val_res);
    }

    return 0;
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
                for (const auto& inc : entry.include_dirs) search_dirs.push_back(inc);
            }
        }
    }

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

int handle_diff_impact(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: compileforge diff-impact <baseline_impact.json> <current_impact.json>\n";
        return 1;
    }
    std::string base_path = argv[2];
    std::string curr_path = argv[3];

    std::ifstream f1(base_path), f2(curr_path);
    if (!f1 || !f2) {
        std::cerr << "Error opening impact report files.\n";
        return 1;
    }
    std::string c1((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    std::string c2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());

    auto j1 = JsonValue::parse(c1);
    auto j2 = JsonValue::parse(c2);
    if (j1.is_error() || j2.is_error()) {
        std::cerr << "Error parsing impact report JSON.\n";
        return 1;
    }

    int risk1 = j1.value()["risk"]["total_risk_score"].as_int();
    int risk2 = j2.value()["risk"]["total_risk_score"].as_int();
    int delta = risk2 - risk1;

    std::cout << "CHANGE RISK DELTA: " << (delta >= 0 ? "+" : "") << delta << " (Base: " << risk1 << " -> Current: " << risk2 << ")\n";
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
        std::cout << "CompileForge v1.0.0 (C++20 Build Intelligence & Change-Impact Toolkit)\n";
        return 0;
    } else if (command == "impact") {
        return handle_impact(argc, argv);
    } else if (command == "validate") {
        return handle_validate(argc, argv);
    } else if (command == "analyze") {
        return handle_analyze(argc, argv);
    } else if (command == "diff") {
        return handle_diff(argc, argv);
    } else if (command == "diff-impact") {
        return handle_diff_impact(argc, argv);
    } else if (command == "init") {
        return handle_init(argc, argv);
    } else {
        std::cerr << "Unknown command: " << command << "\n";
        print_usage();
        return 1;
    }
}
