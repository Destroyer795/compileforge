# Getting Started with CompileForge

This guide walks you through installing CompileForge, running your first whole-project analysis, predicting change impact from Git diffs, and validating predictions against compiler build logs.

---

## 1. Prerequisites

CompileForge requires:
- **C++20 Compiler**: GCC 11+, Clang 13+, or MSVC 2019+ (16.10+)
- **Build System**: CMake 3.20+ and Ninja or Make
- **Git**: Git 2.20+ (for Git change-impact analysis)

No external libraries, package managers (`vcpkg`, `conan`), or runtime packages are needed.

---

## 2. Installation & Building

```bash
# 1. Clone the repository
git clone https://github.com/Destroyer795/compileforge.git
cd compileforge

# 2. Configure and build
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 3. Verify installation with the automated test suite
./build/compileforge_tests
```

Optionally, add `build/` to your `PATH` or install the binary to your system prefix.

---

## 3. First Whole-Project Analysis

To inspect your codebase's include topology, identify circular dependencies, and compute a Build Health Score:

```bash
# Ensure compile_commands.json is available in project root or build/
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Run whole-project analysis
./build/compileforge analyze . --format html --output health_report.html
```

### Example Terminal Output
```
==========================================================
              COMPILEFORGE BUILD HEALTH REPORT            
==========================================================

PROJECT METRICS
  Total Files Scanned:         260
  Translation Units (TUs):     100
  Header Files:                160
  Direct #include Edges:       520
  Circular Include Loops:      0

BUILD HEALTH SCORE: 92 / 100 (HEALTHY)
```

Open `health_report.html` in any browser to view the interactive report.

---

## 4. First Change-Impact Prediction

Before merging a branch or rebuilding, estimate what translation units will be affected by your changes:

```bash
# Predict impact of uncommitted changes
./build/compileforge impact .

# Predict impact across a Git revision range (e.g. against main)
./build/compileforge impact . origin/main..HEAD --format json --output prediction.json
```

### Example Output
```
==========================================================
             COMPILEFORGE CHANGE-IMPACT ANALYSIS          
==========================================================

CHANGE RISK SCORE: 38 / 100 (CRITICAL IMPACT)

CHANGE SUMMARY
  Changed Files:               1
  Potentially Affected TUs:    3
  Potentially Affected Headers: 2
  Estimated Rebuild Surface:   100.0%
  Max Impact Depth:            2 levels

WHY THIS CHANGE IS RISKY
  - affects 3 translation unit(s) (100.0% of project)

REVIEW HOTSPOTS (HIGH INSPECTION PRIORITY)
  include/core/types.hpp
```

---

## 5. First Prediction Validation

After running your build, validate what CompileForge predicted against what the compiler actually rebuilt:

```bash
# 1. Run your build tool while capturing compiler output
ninja -v > build.log 2>&1

# 2. Validate prediction against the build log
./build/compileforge validate prediction.json --log build.log --format html --output validation_report.html
```

### Example Output
```
==========================================================
             CHANGE IMPACT PREDICTION VALIDATION          
==========================================================

PREDICTION VS OBSERVATION
  Predicted Affected TUs:      3 [ESTIMATED]
  Observed Rebuilt TUs:        3 [OBSERVED]
  True Positives (Correct):    3
  False Positives (Over-pred): 0
  False Negatives (Missed):    0

ACCURACY METRICS
  Prediction Precision:        100.0%
  Prediction Recall:           100.0%
  Rebuild Surface Error Delta: 0.0%
  Build-Cost Error Delta:      UNAVAILABLE (No historical build timings)
  Overall Accuracy Rating:     EXCELLENT
==========================================================
```

---

## 6. Basic Troubleshooting

| Symptom | Cause | Solution |
| :--- | :--- | :--- |
| `Compilation database not found` | Missing `compile_commands.json` | Run `cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON` or pass `--db path/to/compile_commands.json` |
| `Failed to execute git diff` | Branch reference not found | Run `git fetch origin` or omit the revision range to analyze uncommitted changes |
| `Observed Rebuilt TUs: 0` | Build log lacked compiler commands | Use verbose compiler output: `ninja -v > build.log 2>&1` |

---

## Next Steps

- [System Architecture](ARCHITECTURE.md) — Subsystem layers and data flow.
- [Change-Impact Engine](CHANGE_IMPACT.md) — Propagation algorithms, risk models, and metrics.
- [CLI Reference](CLI.md) — Complete command options, configuration schemas, and report formats.
- [Validation Guide](VALIDATION.md) — Accuracy formulations and evaluation methodology.
- [Known Limitations](LIMITATIONS.md) — Technical boundaries and scope.
- [Developer Guide](DEVELOPMENT.md) — Building, testing, CI integration, and benchmarks.
