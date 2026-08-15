# CompileForge Developer & Contributor Guide

This guide details the repository layout, build commands, testing procedures, and architectural conventions for developers contributing to CompileForge.

---

## 1. Repository Layout

```
CompileForge/
├── include/compileforge/     # Public C++20 Header API
│   ├── analysis/             # Hotspot, TU cost, health score, regression analyzers
│   ├── core/                 # Result<T>, JsonValue, string/path utils, types
│   ├── git/                  # Git diff parser and revision resolver
│   ├── graph/                # DependencyGraph, CycleDetector (Tarjan SCC)
│   ├── impact/               # ImpactAnalyzer (BFS), RiskScorer (0-100 heuristic)
│   ├── parser/               # CompilationDatabase, CompilerInvocation, IncludeParser
│   ├── project/              # ProjectScanner directory discovery
│   ├── reporting/            # Terminal, JSON, HTML, Impact, and Validation reporters
│   └── validation/           # ImpactValidator, BuildObserver, prediction models
├── src/                      # Source implementation files (.cpp)
├── tests/                    # Unit, integration, invariant, and property tests
│   ├── unit/                 # Subsystem unit tests
│   └── integration/          # End-to-end multi-module integration tests
├── benchmarks/               # Performance benchmark runner
├── examples/                 # Self-contained demo fixtures
│   ├── impact_demo_app/      # Multi-module change-impact demonstration fixture
│   └── synthetic_large_project/ # 200-file multi-tier benchmark fixture
├── evaluations/              # Real-world external evaluation harness
├── docs/                     # Technical architecture, guides, and audits
├── CMakeLists.txt            # Root CMake build definition
└── README.md                 # Product overview and front door
```

---

## 2. Building & Testing

### Prerequisites
- C++20 compliant compiler (GCC 11+, Clang 13+, or MSVC 2019+)
- CMake 3.20+
- Ninja or Make

### Clean Build Commands
```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Run Tests & Benchmarks
```bash
# Execute 41 automated tests
./build/compileforge_tests

# Execute performance benchmarks
./build/compileforge_benchmarks
```

---

## 3. Engineering Conventions

1. **Zero External Runtime Dependencies**: Standard ISO C++20 and STL only. Do not introduce third-party libraries (e.g. Boost, nlohmann/json, fmt).
2. **Explicit Error Handling**: Use `compileforge::Result<T>` rather than throwing C++ exceptions across module boundaries.
3. **Deterministic Output**: All dependency lists, node sets, and JSON properties must maintain sorted, deterministic ordering.
4. **Honest Metrics**: Tag all output metrics explicitly (`MEASURED`, `ESTIMATED`, `OBSERVED`, `HEURISTIC`, `UNAVAILABLE`).
