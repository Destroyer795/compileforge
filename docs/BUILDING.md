# Building CompileForge

CompileForge is designed to build with **zero external runtime dependencies** on any platform supporting C++20.

## Prerequisites

- **C++20 Compiler**: GCC 11+, Clang 13+, or MSVC 2019+ (16.10+)
- **Build Generator**: CMake 3.20+ and Make or Ninja
- **Version Control**: Git 2.20+ (optional, for Git churn analysis features)

## Zero-Dependency Architecture

CompileForge is fully self-contained:
- **JSON Engine**: Handcrafted zero-dependency fast JSON tokenizer, parser, and serializer in `compileforge::JsonValue`.
- **Testing Framework**: Embedded macro-driven test suite runner in `tests/test_framework.hpp`.
- **HTML Dashboard**: Self-contained HTML/CSS/JS single-file report generator embedded in `compileforge::HtmlReporter`.

No external package managers (`vcpkg`, `conan`, `apt-get`, `npm`) are required to build or run CompileForge.

## Building from Source

### Quick Start (Linux / macOS / Windows)

```bash
# 1. Clone repository
git clone https://github.com/Destroyer795/compileforge.git
cd compileforge

# 2. Configure build directory
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build target binary and tests
cmake --build build --config Release

# 4. Run unit and integration tests
ctest --test-dir build --output-on-failure
# Or run test executable directly:
./build/compileforge_tests
```

### Build Options

| Option | Default | Description |
| :--- | :--- | :--- |
| `COMPILEFORGE_BUILD_TESTS` | `ON` | Build unit and integration test runner (`compileforge_tests`) |
| `COMPILEFORGE_BUILD_BENCHMARKS` | `ON` | Build benchmark suite runner (`compileforge_benchmarks`) |
| `COMPILEFORGE_ENABLE_WARNINGS_AS_ERRORS` | `OFF` | Treat compiler warnings as build errors |

### Running Example Analyses

```bash
# Analyze sample project
./build/compileforge analyze ./examples/monolith_app --output report.html --format html

# Detect circular dependencies and fail CI if cycles found
./build/compileforge analyze ./examples/circular_includes --fail-on-cycle

# Compare baseline performance against current run
./build/compileforge diff baseline.json current.json
```
