# CompileForge

## Know the likely cost of a C++ change before you merge it.

[![CI Pipeline](https://github.com/Destroyer795/compileforge/actions/workflows/ci.yml/badge.svg)](https://github.com/Destroyer795/compileforge/actions)
[![Language](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

**CompileForge** is a zero-runtime-dependency C++ build intelligence and change-impact analysis toolkit. It connects Git revision diffs with C++ dependency analysis, compilation databases, and code metrics to answer:

- **What may be affected?** Traces forward dependents from modified headers to translation units.
- **How large is the rebuild surface?** Estimates the exact percentage of project translation units affected.
- **Which files deserve code review?** Ranks review hotspots by architectural centrality and blast radius.
- **How risky is the change?** Computes an explainable 0–100 Change Risk Score with data-backed reasons.
- **What actually happened?** Validates predictions against real compiler build logs (`compileforge validate`).

```
  DETECT (Git Diff)  ──>  PREDICT (Impact & Risk)  ──>  BUILD (Compiler)  ──>  VERIFY (Validate Accuracy)
```

---

## Core Differentiators

CompileForge combines:
1. **Git-Aware Change-Impact Workflow**: Traces commit diffs (`HEAD~1..HEAD`, `main..HEAD`) and uncommitted changes.
2. **Forward Dependency Propagation**: Evaluates inclusion graphs to identify transitively affected translation units.
3. **Rebuild-Surface Estimation**: Quantifies the percentage of project source code and TUs impacted by a change.
4. **Explainable Risk Scoring**: Deterministic 0–100 heuristic scoring based on blast radius, centrality, churn, and complexity.
5. **Validation Against Observed Builds**: Compares predicted impact against observed compiler build logs.
6. **Zero Runtime Dependencies**: Handcrafted C++20 engine without external JSON, AST, or graph libraries.
7. **Native CI Integration**: Automated risk thresholds (`--fail-on-risk <threshold>`) and machine-readable JSON reports.

---

## What CompileForge Is / Is Not

### CompileForge IS:
- **Zero Third-Party Runtime Dependencies**: Pure ISO C++20 standard library.
- **Git-Aware**: Evaluates commit ranges (`HEAD~1..HEAD`, `main..HEAD`) and working tree diffs.
- **Change-Impact Focused**: Designed to evaluate the build cost of code modifications before merging.
- **CI-Ready**: Native `--fail-on-risk <threshold>` gate and JSON/HTML reporting.
- **Transparent & Explainable**: Every risk point and accuracy percentage is backed by analyzed data.

### CompileForge IS NOT:
- **A full C++ compiler**: It uses fast static lexical preprocessor analysis (>5.3M lines/sec), not a semantic compiler AST.
- **A replacement for compiler profilers**: Does not replace specialized tools like Microsoft C++ Build Insights or Clang `-ftime-trace`.
- **A guarantee of build system behavior**: Preprocessor estimates may differ from compiler caching/PCH implementations.
- **A software defect predictor**: Risk scores reflect architectural blast radius and build cost, not code bug probabilities.

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
# 1. Predict impact and save to JSON
./build/compileforge impact HEAD~1..HEAD --format json --output prediction.json

# 2. Run in CI mode: fail PR if Change Risk Score exceeds 70
./build/compileforge impact HEAD~1..HEAD --fail-on-risk 70

# 3. Validate impact predictions against actual build logs
./build/compileforge validate prediction.json --log build.log

# 4. Analyze whole-project dependency graph and compilation database
./build/compileforge analyze . --format html --output report.html
```

---

## Documentation

- [Product Demo & Walkthrough](docs/demo.md)
- [Change-Impact Validation Case Study](docs/case-study.md)
- [Reproducible Evaluation Specification](docs/evaluation.md)
- [Architecture & Design Specification](docs/ARCHITECTURE.md)
- [Metrics & Scoring Methodology](docs/METRICS.md)
- [Independent Commercial Buyer Audit](docs/BUYER_AUDIT.md)
- [Final Phase 5 Value Audit](docs/FINAL_VALUE_AUDIT.md)
- [Configuration Schema (.compileforge.json)](docs/CONFIGURATION.md)
- [Continuous Integration Guide](docs/CI.md)
- [Real-World Evaluation Harness](evaluations/README.md)

---

## License

CompileForge is released under the [MIT License](LICENSE).
