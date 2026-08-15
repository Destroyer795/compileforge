#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include <compileforge/core/result.hpp>
#include <compileforge/core/types.hpp>
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
#include <compileforge/recommendations/recommendation_engine.hpp>
#include <compileforge/config/config.hpp>
#include <compileforge/cache/analysis_cache.hpp>
#include <compileforge/reporting/report.hpp>

using namespace compileforge;

static void print_usage() {
    std::cout << R"(CompileForge - C++ Build Intelligence & Architecture Toolkit v1.0.0

USAGE:
  compileforge analyze [target_dir] [options]
  compileforge diff <baseline.json> <current.json>
  compileforge init [target_dir]
  compileforge --help
  compileforge --version

OPTIONS:
  -c, --compilation-database <path>  Path to compile_commands.json
  -o, --output <path>                Output report file path
  -f, --format <terminal|json|html>  Report format (default: terminal)
  --config <path>                    Path to .compileforge.json
  --fail-on-cycle                    Return exit code 1 if circular dependencies found
  --threshold <score>                Max allowed hotspot score (default: 80.0)
)";
}

static int handle_analyze(const std::vector<std::string>& args) {
    std::string target_dir = ".";
    std::string comp_db_path;
    std::string output_path;
    std::string format = "terminal";
    std::string config_path;
    bool fail_on_cycle = false;
    double threshold = 80.0;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "-c" || arg == "--compilation-database") {
            if (i + 1 < args.size()) comp_db_path = args[++i];
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < args.size()) output_path = args[++i];
        } else if (arg == "-f" || arg == "--format") {
            if (i + 1 < args.size()) format = args[++i];
        } else if (arg == "--config") {
            if (i + 1 < args.size()) config_path = args[++i];
        } else if (arg == "--fail-on-cycle") {
            fail_on_cycle = true;
        } else if (arg == "--threshold") {
            if (i + 1 < args.size()) threshold = std::stod(args[++i]);
        } else if (!utils::starts_with(arg, "-")) {
            target_dir = arg;
        }
    }

    target_dir = utils::normalize_path(target_dir);
    if (!std::filesystem::exists(target_dir)) {
        std::cerr << "Error: Target directory does not exist: " << target_dir << "\n";
        return 1;
    }

    // 1. Load config
    Config cfg = Config::default_config();
    if (!config_path.empty()) {
        auto cfg_res = Config::load_file(config_path);
        if (cfg_res.has_value()) cfg = cfg_res.value();
    } else {
        std::filesystem::path cfg_file = std::filesystem::path(target_dir) / ".compileforge.json";
        if (std::filesystem::exists(cfg_file)) {
            auto cfg_res = Config::load_file(cfg_file.string());
            if (cfg_res.has_value()) cfg = cfg_res.value();
        }
    }

    // 2. Scan project files
    ScanOptions scan_opts;
    scan_opts.root_path = target_dir;
    scan_opts.ignore_patterns = cfg.exclude_paths;
    ProjectScanner scanner(scan_opts);
    auto scan_res = scanner.scan();
    if (scan_res.is_error()) {
        std::cerr << "Error scanning project: " << scan_res.error().to_string() << "\n";
        return 1;
    }
    auto files = scan_res.value();

    // 3. Load compilation database
    CompilationDatabase comp_db;
    if (comp_db_path.empty()) {
        std::filesystem::path default_db = std::filesystem::path(target_dir) / "compile_commands.json";
        if (std::filesystem::exists(default_db)) {
            comp_db_path = default_db.string();
        }
    }
    if (!comp_db_path.empty()) {
        auto db_res = CompilationDatabase::load_file(comp_db_path);
        if (db_res.has_value()) {
            comp_db = db_res.value();
        }
    }

    // 4. Load cache
    std::filesystem::path cache_file = std::filesystem::path(target_dir) / ".compileforge.cache";
    AnalysisCache cache = AnalysisCache::load(cache_file.string()).value_or(AnalysisCache{});

    // 5. Build Dependency Graph
    DependencyGraph graph;
    std::vector<std::string> search_dirs = cfg.custom_include_dirs;

    for (auto& file : files) {
        const auto* comp_entry = comp_db.find_entry(file.canonical_path);
        std::vector<std::string> file_search_dirs = search_dirs;
        if (comp_entry) {
            file_search_dirs.insert(file_search_dirs.end(), comp_entry->include_dirs.begin(), comp_entry->include_dirs.end());
        }

        const auto* cached = cache.get(file.relative_path, file.content_hash);
        if (cached) {
            file.metrics = cached->metrics;
            file.includes = cached->includes;
        } else {
            auto parse_res = IncludeParser::parse_file(file.canonical_path);
            if (parse_res.has_value()) {
                file.metrics = parse_res.value().metrics;
                file.includes = parse_res.value().includes;
                cache.put(file.relative_path, file.content_hash, file.metrics, file.includes);
            }
        }

        // Resolve includes
        for (auto& inc : file.includes) {
            inc.resolved_path = IncludeParser::resolve_include_path(inc, file.canonical_path, file_search_dirs, target_dir);
            if (!inc.resolved_path.empty()) {
                inc.is_resolved = true;
                std::string rel_inc = utils::to_relative_path(inc.resolved_path, target_dir);
                graph.add_edge(file.relative_path, rel_inc);
            }
        }

        graph.add_node(file);
    }

    // Save cache
    (void)cache.save(cache_file.string());

    // 6. Graph & Metrics computation
    graph.compute_fan_stats();
    BuildTraceAnalyzer::estimate_build_times(graph);
    GitAnalyzer::enrich_graph(graph, target_dir);
    HotspotScorer::compute_hotspots(graph);

    // 7. Cycle detection
    auto cycles = CycleDetector::detect_cycles(graph);

    // 8. Summary & Recommendations
    ProjectSummary summary = SourceMetrics::compute_summary(graph, cycles.size());
    auto recommendations = RecommendationEngine::generate_recommendations(graph, cycles);

    // Build Report Object
    AnalysisReport report;
    report.summary = summary;
    report.cycles = cycles;
    report.recommendations = recommendations;
    report.top_hotspots = HotspotScorer::get_top_hotspots(graph, 10);

    for (const auto& path : graph.all_nodes()) {
        const auto* n = graph.get_node(path);
        if (n) report.files.push_back(*n);
    }

    std::sort(report.files.begin(), report.files.end(), [](const FileNode& a, const FileNode& b) {
        return a.fan_stats.fan_in_transitive > b.fan_stats.fan_in_transitive;
    });

    for (size_t i = 0; i < std::min<size_t>(5, report.files.size()); ++i) {
        if (report.files[i].kind == FileKind::Header) {
            report.top_fanin_headers.push_back(report.files[i]);
        }
    }

    // 9. Output generation
    if (format == "json") {
        std::string json_output = JsonReporter::render(report, 2);
        if (!output_path.empty()) {
            std::ofstream ofs(output_path);
            ofs << json_output;
            std::cout << "JSON report saved to: " << output_path << "\n";
        } else {
            std::cout << json_output << "\n";
        }
    } else if (format == "html") {
        std::string html_output = HtmlReporter::render(report, target_dir);
        if (output_path.empty()) output_path = "compileforge_report.html";
        std::ofstream ofs(output_path);
        ofs << html_output;
        std::cout << "HTML report saved to: " << output_path << "\n";
    } else {
        TerminalReporter::print(report, true);
    }

    if (fail_on_cycle && !cycles.empty()) {
        std::cerr << "CI Failure: " << cycles.size() << " circular dependency loop(s) detected.\n";
        return 1;
    }

    for (const auto& spot : report.top_hotspots) {
        if (spot.hotspot.total_score > threshold) {
            std::cout << "Warning: Hotspot score for " << spot.relative_path << " (" << spot.hotspot.total_score << ") exceeds threshold (" << threshold << ").\n";
        }
    }

    return 0;
}

static int handle_diff(const std::string& baseline_path, const std::string& current_path) {
    auto res = RegressionDetector::compare_files(baseline_path, current_path);
    if (res.is_error()) {
        std::cerr << "Diff Error: " << res.error().to_string() << "\n";
        return 1;
    }

    const auto& report = res.value();
    if (!report.has_regressions) {
        std::cout << "SUCCESS: No build regressions detected between baseline and current runs.\n";
        return 0;
    }

    std::cout << "REGRESSIONS DETECTED (" << report.deltas.size() << " issues):\n";
    for (const auto& delta : report.deltas) {
        std::cout << "  [" << delta.category << "] " << delta.file << ": " << delta.message << "\n";
    }
    return 1;
}

static int handle_init(const std::string& target_dir) {
    std::filesystem::path cfg_path = std::filesystem::path(target_dir) / ".compileforge.json";
    if (std::filesystem::exists(cfg_path)) {
        std::cout << ".compileforge.json already exists in " << target_dir << "\n";
        return 0;
    }

    Config cfg = Config::default_config();
    JsonValue::ObjectType root;
    JsonValue::ArrayType exc;
    for (const auto& p : cfg.exclude_paths) exc.push_back(p);
    root["exclude_paths"] = exc;
    root["max_hotspot_score_threshold"] = cfg.max_hotspot_score_threshold;
    root["fail_on_cycles"] = cfg.fail_on_cycles;

    std::ofstream ofs(cfg_path);
    ofs << JsonValue(root).serialize(2);
    std::cout << "Initialized .compileforge.json in " << target_dir << "\n";
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 0;
    }

    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    std::string command = args[0];
    if (command == "--help" || command == "-h" || command == "help") {
        print_usage();
        return 0;
    }
    if (command == "--version" || command == "-v" || command == "version") {
        std::cout << "CompileForge v1.0.0 (C++20)\n";
        return 0;
    }

    if (command == "analyze") {
        std::vector<std::string> sub_args(args.begin() + 1, args.end());
        return handle_analyze(sub_args);
    }
    if (command == "diff") {
        if (args.size() < 3) {
            std::cerr << "Usage: compileforge diff <baseline.json> <current.json>\n";
            return 1;
        }
        return handle_diff(args[1], args[2]);
    }
    if (command == "init") {
        std::string target = (args.size() >= 2) ? args[1] : ".";
        return handle_init(target);
    }

    // Default to analyze command if positional argument is directory
    return handle_analyze(args);
}
