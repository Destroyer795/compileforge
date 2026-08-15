# CompileForge Testing & Quality Assurance

CompileForge maintains a zero-external-dependency test suite verifying algorithmic invariants, data structures, parsing robustness, and end-to-end workflows.

---

## 1. Test Suite Overview

CompileForge executes **41 automated test cases** covering:

- **Unit Tests**:
  - `test_json`: JSON primitive parsing, serialization, and malformed input handling.
  - `test_scanner`: Directory traversal, file kind detection, and ignore rules.
  - `test_compilation_database`: Clang compilation database ingestion and flag extraction.
  - `test_compiler_invocation`: Tokenization of `-std=`, `-O`, `-I`, `-D`, and warnings.
  - `test_include_parser`: Directive parsing, `#pragma once`, include guards, and `#if 0` skipping.
  - `test_dependency_graph`: Node registration, fan-in/fan-out statistics, and graph duality.
  - `test_cycle_detector`: Tarjan SCC cycle identification and circular include reporting.
  - `test_source_metrics`: LOC, comments, preprocessor count, and cyclomatic complexity.
  - `test_hotspot`: Multi-variate hotspot ranking and risk categorization.
  - `test_build_config_health`: Detection of flag mismatches and inconsistent standards.
  - `test_tu_cost`: Translation unit cost classification and memory/CPU heuristics.
  - `test_build_health_score`: 0–100 weighted health score computation.
  - `test_impact_analyzer`: Forward dependent propagation and rebuild surface calculations.
  - `test_risk_scorer`: Determinism, monotonicity, and boundary testing of 0–100 risk score.
  - `test_git_diff`: Revision diff parsing, rename detection, and non-repo resilience.
  - `test_validation_model`: True/false positives, precision, recall, and build log parsing.
  - `test_impact_invariants`: Graph reachability invariants for all reported affected nodes.
- **Integration Tests**:
  - `test_e2e_monolith_app_analysis`: Full pipeline run on synthetic monolith fixture.
  - `test_e2e_circular_includes_detection`: Verification of cycle detection on complex graph.
- **Robustness Tests**:
  - `test_malformed_inputs`: Graceful error recovery on corrupt JSON, missing databases, and invalid paths.

---

## 2. Test Execution

```bash
# Build and run the full test suite
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/compileforge_tests
```

**Verified Test Result**: `41/41 tests executed in ~215 ms (100% pass rate)`.
