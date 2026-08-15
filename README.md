# CompileForge - C++ Build Intelligence & Optimization Toolkit

[![CI Pipeline](https://github.com/Destroyer795/compileforge/actions/workflows/ci.yml/badge.svg)](https://github.com/Destroyer795/compileforge/actions)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

**CompileForge** is a production-quality, cross-platform C++20 developer tool and build intelligence engine. It analyzes compilation databases (`compile_commands.json`), include dependency graphs, compiler invocation flags, source metrics, and Git history to identify build bottlenecks, circular inclusion loops, and actionable refactoring opportunities.

> [!NOTE]
> **Static Lexical Analysis Disclaimer**: CompileForge performs fast static lexical analysis of source files and compilation databases. It does NOT invoke a full compiler C++ AST or semantic preprocessor parser. All unused header predictions or complexity indices are lexical estimates.

---

## Key Features

- **Compiler Invocation Analysis**: Parses GCC, Clang, and MSVC compiler flags (`-std=c++20`, `-O2`, `-I`, `-isystem`, `-D`, `-U`, `-flto`, `-fsanitize=address`).
- **Build Configuration Health**: Audits cross-TU flag consistency, detecting mixed optimization levels, mismatched C++ standards, and missing warning flags.
- **Circular Include Loop Detection**: Uses Tarjan's Strongly Connected Components algorithm to identify circular inclusion loops (`A.hpp -> B.hpp -> C.hpp -> A.hpp`) in **<0.1 ms**.
- **Translation Unit Cost Model**: Classifies TUs into cost tiers (`LOW`, `MEDIUM`, `HIGH`, `CRITICAL`) based on transitive header load, macro density, and template counts.
- **Build Health Score (0–100)**: Multi-variate rating evaluating repository build configuration quality and dependency health.
- **Prioritized Action Plan**: Generates ordered refactoring steps (`PRIORITY 1`, `PRIORITY 2`, `PRIORITY 3`) with impact estimates and step-by-step guidance.
- **Report Diffing (`compileforge diff`)**: Compares baseline vs current runs to catch regressions in CI pipelines.
- **Multi-Format Output**: Rich ANSI terminal cards, schema 1.0 JSON export, and single-file dark mode HTML dashboard with filterable search tables.

---

## Quick Start

### 1. Build Requirements

- C++20 compliant compiler (GCC 10+, Clang 11+, or MSVC 2019+)
- CMake 3.20+
- Ninja or Make

### 2. Build Instructions

```bash
git clone https://github.com/Destroyer795/compileforge.git
cd compileforge
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### 3. Usage Examples

```bash
# Analyze a project directory (reads compile_commands.json)
./build/compileforge analyze ./examples/synthetic_large_project

# Export interactive single-file HTML report dashboard
./build/compileforge analyze . --format html --output report.html

# Run in CI mode: fail build if circular include loops exist
./build/compileforge analyze . --fail-on-cycle --fail-on-hotspot --format json --output report.json

# Compare baseline vs current run for regressions
./build/compileforge diff baseline.json report.json
```

---

## Documentation

- [Architecture & Design Specification](docs/ARCHITECTURE.md)
- [Metrics & Scoring Methodology](docs/METRICS.md)
- [Configuration Schema (.compileforge.json)](docs/CONFIGURATION.md)
- [Continuous Integration Guide](docs/CI.md)
- [Building & Dependency Rules](docs/BUILDING.md)

---

## License

CompileForge is released under the [MIT License](LICENSE).
