# Contributing to CompileForge

Thank you for your interest in contributing to CompileForge.

---

## Development Principles

1. **Zero External Runtime Dependencies**: Standard ISO C++20 and STL only. Do not introduce third-party libraries (e.g. Boost, nlohmann/json, fmt).
2. **Explicit Error Handling**: Use `compileforge::Result<T>` rather than throwing C++ exceptions across module boundaries.
3. **Deterministic Output**: All dependency lists, node sets, and JSON properties must maintain sorted, deterministic ordering.
4. **Honest Metrics**: Tag all output metrics explicitly (`MEASURED`, `ESTIMATED`, `OBSERVED`, `HEURISTIC`, `UNAVAILABLE`).

---

## Getting Started

### Prerequisites
- C++20 compliant compiler (GCC 11+, Clang 13+, or MSVC 2019+)
- CMake 3.20+
- Ninja or Make
- Git

### Build & Run Tests
```bash
# Clone the repository
git clone https://github.com/Destroyer795/compileforge.git
cd compileforge

# Configure and compile with Ninja
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Execute automated tests
ctest --test-dir build --output-on-failure
# Or run test executable directly:
./build/compileforge_tests
```

---

## Submitting Pull Requests

1. Fork the repository and create a feature branch from `main`.
2. Ensure your changes compile cleanly without warnings on GCC, Clang, or MSVC.
3. Add unit or integration tests for new functionality in `tests/unit/` or `tests/integration/`.
4. Ensure all 41 test cases pass.
5. Keep commit messages clear, descriptive, and focused on the change rationale.
6. Open a Pull Request against `main`.

---

## Code Style

- Standard ISO C++20.
- 4 spaces indentation, no tabs.
- Class names in `PascalCase`, functions and variables in `snake_case`, constants in `UPPER_SNAKE_CASE`.
- All public headers reside in `include/compileforge/` with `#pragma once`.
