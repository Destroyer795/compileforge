#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <iomanip>

#include <compileforge/core/json.hpp>
#include <compileforge/parser/include_parser.hpp>
#include <compileforge/graph/dependency_graph.hpp>
#include <compileforge/graph/cycle_detector.hpp>

using namespace compileforge;

static void benchmark_json_parsing() {
    std::string large_json = R"([)";
    for (int i = 0; i < 5000; ++i) {
        large_json += R"({"directory":"/build","command":"g++ -Iinclude -c src/file.cpp","file":"src/file.cpp","output":"file.o"})";
        if (i + 1 < 5000) large_json += ",";
    }
    large_json += "]";

    auto start = std::chrono::high_resolution_clock::now();
    size_t iterations = 20;
    for (size_t i = 0; i < iterations; ++i) {
        auto res = JsonValue::parse(large_json);
        (void)res;
    }
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    double total_bytes = static_cast<double>(large_json.size() * iterations);
    double mb_per_sec = (total_bytes / (1024.0 * 1024.0)) / (elapsed_ms / 1000.0);

    std::cout << "[ BENCHMARK ] JSON Parser Throughput:  " << std::fixed << std::setprecision(2)
              << mb_per_sec << " MB/s (" << elapsed_ms << " ms total for " << iterations << " iterations)\n";
}

static void benchmark_include_parser() {
    std::string mock_code;
    for (int i = 0; i < 1000; ++i) {
        mock_code += "#include \"header_" + std::to_string(i % 50) + ".hpp\"\n";
        mock_code += "int fn_" + std::to_string(i) + "() { if (true) return " + std::to_string(i) + "; }\n";
    }

    auto start = std::chrono::high_resolution_clock::now();
    size_t iterations = 50;
    size_t total_lines = 2000 * iterations;
    for (size_t i = 0; i < iterations; ++i) {
        auto res = IncludeParser::parse_content(mock_code, "test.cpp");
        (void)res;
    }
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    double lines_per_sec = (static_cast<double>(total_lines) / (elapsed_ms / 1000.0));

    std::cout << "[ BENCHMARK ] Include Parser Speed:    " << std::fixed << std::setprecision(0)
              << lines_per_sec << " lines/sec (" << elapsed_ms << " ms total)\n";
}

static void benchmark_cycle_detection() {
    DependencyGraph graph;
    constexpr int N = 100;
    for (int i = 0; i < N; ++i) {
        FileNode node;
        node.relative_path = "header_" + std::to_string(i) + ".hpp";
        node.kind = FileKind::Header;
        graph.add_node(node);
    }
    for (int i = 0; i < N; ++i) {
        graph.add_edge("header_" + std::to_string(i) + ".hpp", "header_" + std::to_string((i + 1) % N) + ".hpp");
    }

    auto start = std::chrono::high_resolution_clock::now();
    size_t iterations = 100;
    for (size_t i = 0; i < iterations; ++i) {
        auto cycles = CycleDetector::detect_cycles(graph);
        (void)cycles;
    }
    auto end = std::chrono::high_resolution_clock::now();

    double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    std::cout << "[ BENCHMARK ] Cycle Detector Speed:    " << std::fixed << std::setprecision(3)
              << (elapsed_ms / static_cast<double>(iterations)) << " ms/run (100-node graph)\n";
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "               COMPILEFORGE BENCHMARKS                  \n";
    std::cout << "========================================================\n";
    benchmark_json_parsing();
    benchmark_include_parser();
    benchmark_cycle_detection();
    std::cout << "========================================================\n";
    return 0;
}
