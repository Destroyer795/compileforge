# CompileForge - C++ Build Intelligence & Change-Impact Toolkit

[![CI Pipeline](https://github.com/Destroyer795/compileforge/actions/workflows/ci.yml/badge.svg)](https://github.com/Destroyer795/compileforge/actions)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

**CompileForge** is a zero-dependency C++ build intelligence and change-impact analysis toolkit. It answers the fundamental question: **"What will this code change cost?"**

By connecting Git revision diffs to C++ preprocessor dependency graphs, CompileForge calculates estimated rebuild surfaces, build cost projections, risk scores (0–100), and ranks review hotspots to guide code reviews and CI risk gates.

> [!NOTE]
> **Static Lexical Analysis Disclaimer**: CompileForge performs fast static lexical analysis of source files, inclusion directives, and compilation databases. It does NOT invoke a full compiler C++ AST or semantic preprocessor parser. Rebuild surfaces and risk scores represent static dependency estimates.

---

## Core Capabilities

- **Change-Impact Analysis (`compileforge impact`)**: Traces Git diffs (`HEAD~1..HEAD`, `main..HEAD`, or working tree edits) to calculate potentially affected translation units and headers.
- **Estimated Rebuild Surface**: Computes the exact percentage of project translation units and source lines of code affected by header modifications.
- **Change Risk Score (0–100)**: Deterministic, explainable risk scoring breaking down impact, build cost, dependency centrality, complexity, churn, and inclusion cycles.
- **"Why This Change Is Risky"**: Data-driven explanation lists detailing why a change carries review risk.
- **Review Hotspot Ranking**: Ranks affected files that warrant focused human code review attention.
- **CI Gate Mode (`--fail-on-risk <threshold>`)**: Acts as a change-risk gate in CI pipelines to prevent high-blast-radius modifications from merging silently.
- **Compiler Invocation Analysis**: Parses GCC, Clang, and MSVC flags (`-std=c++20`, `-O2`, `-I`, `-isystem`, `-D`, `-U`, `-flto`, `-fsanitize=address`).
- **Circular Include Loop Detection**: Uses Tarjan's Strongly Connected Components algorithm to find circular inclusion loops (`A.hpp -> B.hpp -> C.hpp -> A.hpp`) in **<0.1 ms**.

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
# Analyze change-impact of the latest commit
./build/compileforge impact HEAD~1..HEAD

# Analyze change-impact of modifying a specific header file
./build/compileforge impact --file include/network/types.hpp

# Run in CI mode: fail PR if Change Risk Score exceeds 70
./build/compileforge impact HEAD~1..HEAD --fail-on-risk 70 --format json --output impact_report.json

# Analyze whole-project build health & compile database
./build/compileforge analyze ./examples/synthetic_large_project

# Export interactive single-file HTML change-impact dashboard
./build/compileforge impact HEAD~1..HEAD --format html --output impact.html
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
