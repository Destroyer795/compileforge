# CompileForge

> **Cross-Platform C++20 Build Intelligence & Optimization Toolkit**

CompileForge is a production-quality C++20 developer tool that analyzes compilation databases (`compile_commands.json`), C/C++ source trees, include/dependency relationships, code complexity metrics, build traces, and Git history to identify build bottlenecks, architectural hotspots, and actionable optimization opportunities.

---

## Key Features

- **Project & Compilation Database Ingestion**: Parses `compile_commands.json` (GCC, Clang, MSVC syntax), normalizes include directories, defines, and compiler flags.
- **Preprocessor Include & Graph Analysis**: Fast stream-based lexer extracts `#include "..."` and `#include <...>` directives, header guards (`#pragma once`, `#ifndef`), and builds directed dependency graphs.
- **Tarjan Circular Dependency Detection**: Detects include loops (e.g. `A.hpp -> B.hpp -> C.hpp -> A.hpp`) and outputs cycle traces.
- **Fan-In / Fan-Out Analysis**: Computes direct and transitive dependencies, transitive dependents, and compilation blast radius.
- **Multi-Factor Hotspot Scoring**: Normalizes compilation size, fan-in impact, cyclomatic complexity, and Git churn into a 0–100 Hotspot Score.
- **Actionable Optimization Recommendations**: Prioritized recommendations (HIGH/MED/LOW) suggesting header splits, Pimpl pattern conversions, forward declarations, and guard fixes.
- **Multi-Format Reporting**:
  - **Terminal**: ANSI-colored formatted tables, summaries, and warning cards.
  - **JSON**: Machine-readable schema export for CI/CD integration.
  - **HTML**: Standalone interactive single-file dashboard with dark mode design and SVG visualizations.
- **Regression Detection**: Compare baseline vs. current analysis runs (`compileforge diff`) to block build degradation in CI.
- **Incremental Disk Cache**: Sub-second re-analysis using `.compileforge.cache` FNV-1a content hashing.
- **Zero Runtime Dependencies**: Completely self-contained C++20 implementation.

---

## Quick Start

### 1. Build from Source

```bash
git clone https://github.com/Destroyer795/compileforge.git
cd compileforge

# Configure and build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Run tests
ctest --test-dir build --output-on-failure
```

### 2. Run Analysis on Example Project

```bash
# Analyze example project and output interactive HTML report
./build/compileforge analyze ./examples/monolith_app --format html --output report.html

# Run circular dependency detection in CI mode
./build/compileforge analyze ./examples/circular_includes --fail-on-cycle
```

---

## Terminal Report Example

```
==========================================================
               COMPILEFORGE BUILD INTELLIGENCE            
==========================================================

PROJECT SUMMARY
Files:            842
Headers:          317
Translation Units: 525
Lines of Code:    142,850 (SLOC: 104,210)

BUILD HOTSPOTS
  src/render/render.cpp             4.82s  (Score: 84.5)
  src/storage/index.cpp             3.71s  (Score: 78.2)
  src/network/client.cpp            3.42s  (Score: 72.1)

DEPENDENCY HOTSPOTS
  include/core/common.hpp           241 dependents
  include/network/types.hpp         187 dependents

ARCHITECTURAL WARNINGS
  7 circular dependencies
  12 high fan-out headers
  8 high-churn/high-complexity files

RECOMMENDATIONS
  HIGH  Split heavyweight header: common.hpp
  HIGH  Investigate network/types.hpp transitive includes
  MED   Refactor high-churn/high-complexity module
```

---

## CLI Command Reference

```
USAGE:
  compileforge analyze [target_dir] [options]
  compileforge diff <baseline.json> <current.json>
  compileforge init [target_dir]
  compileforge --help
  compileforge --version

OPTIONS:
  -c, --compilation-database <path>  Path to compile_commands.json
  -o, --output <path>                Output report file path (e.g. report.html)
  -f, --format <terminal|json|html>  Report output format (default: terminal)
  --config <path>                    Path to .compileforge.json
  --fail-on-cycle                    Return exit code 1 if circular dependencies exist
  --threshold <score>                Max allowed hotspot score before warning
```

---

## Architecture Overview

CompileForge follows a modular pipeline design:

```
ProjectScanner -> CompilationDatabase -> IncludeParser -> DependencyGraph
                                                                |
                                                      +---------+---------+
                                                      |                   |
                                                CycleDetector       SourceMetrics
                                                      |                   |
                                                      +---------+---------+
                                                                |
                                                          HotspotScorer
                                                                |
                                                       RecommendationEngine
                                                                |
                                                        Reporters (CLI/JSON/HTML)
```

For build detailed instructions, see [docs/BUILDING.md](file:///c:/Users/PRANAV KISHAN/Desktop/forked trash/compileforge/docs/BUILDING.md).

---

## License

CompileForge is licensed under the MIT License.
