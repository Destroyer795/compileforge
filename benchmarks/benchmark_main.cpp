#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <compileforge/core/json.hpp>
#include <compileforge/parser/include_parser.hpp>
#include <compileforge/graph/dependency_graph.hpp>
#include <compileforge/graph/cycle_detector.hpp>
#include <compileforge/project/scanner.hpp>

using namespace compileforge;

int main() {
    std::cout << "========================================================\n";
    std::cout << "               COMPILEFORGE BENCHMARKS                  \n";
    std::cout << "========================================================\n";

    // Benchmark 1: JSON Parsing Speed
    std::string sample_json = R"([)";
    for (int i = 0; i < 1000; ++i) {
        sample_json += R"({"directory": "/build", "command": "g++ -Iinclude -O2 -c src/main.cpp -o main.o", "file": "src/main.cpp"},)";
    }
    sample_json.back() = ']';

    auto start_json = std::chrono::high_resolution_clock::now();
    constexpr int iterations = 20;
    for (int i = 0; i < iterations; ++i) {
        auto res = JsonValue::parse(sample_json);
        (void)res;
    }
    auto end_json = std::chrono::high_resolution_clock::now();
    double json_time_ms = std::chrono::duration<double, std::milli>(end_json - start_json).count();
    double json_mb = (static_cast<double>(sample_json.size() * iterations) / (1024.0 * 1024.0));
    double json_speed = json_mb / (json_time_ms / 1000.0);

    std::cout << "[ BENCHMARK ] JSON Parser Throughput:  " << json_speed << " MB/s (" << json_time_ms << " ms total for " << iterations << " iterations)\n";

    // Benchmark 2: Include Lexer Speed
    std::string mock_code;
    for (int i = 0; i < 5000; ++i) {
        mock_code += "#include <vector>\n#include \"my_header" + std::to_string(i % 10) + ".hpp\"\nint x" + std::to_string(i) + " = 0;\n";
    }

    auto start_lex = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 20; ++i) {
        auto res = IncludeParser::parse_content(mock_code, "mock.cpp");
        (void)res;
    }
    auto end_lex = std::chrono::high_resolution_clock::now();
    double lex_time_ms = std::chrono::duration<double, std::milli>(end_lex - start_lex).count();
    double total_lines = 15000.0 * 20.0;
    double lines_per_sec = total_lines / (lex_time_ms / 1000.0);

    std::cout << "[ BENCHMARK ] Include Parser Speed:    " << static_cast<long long>(lines_per_sec) << " lines/sec (" << lex_time_ms << " ms total)\n";

    // Benchmark 3: Multi-Tier Project Scan Speed
    ScanOptions scan_opts;
    scan_opts.root_path = "examples/synthetic_large_project";
    ProjectScanner scanner(scan_opts);

    auto start_scan = std::chrono::high_resolution_clock::now();
    auto scan_res = scanner.scan();
    auto end_scan = std::chrono::high_resolution_clock::now();
    double scan_time_ms = std::chrono::duration<double, std::milli>(end_scan - start_scan).count();

    size_t scanned_count = scan_res.is_ok() ? scan_res.value().size() : 0;
    std::cout << "[ BENCHMARK ] 200-File Project Scan:   " << scan_time_ms << " ms (" << scanned_count << " files scanned)\n";

    std::cout << "========================================================\n";
    return 0;
}
