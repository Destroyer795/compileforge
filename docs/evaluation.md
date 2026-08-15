# CompileForge Reproducible Evaluation Specification

This document details the exact environment, methodology, commands, and raw results for replicating CompileForge's Change-Impact and Prediction-Validation benchmarks.

---

## 1. Environment Specifications

- **Operating System**: Linux POSIX (Ubuntu 22.04 LTS via WSL2 on Windows 11)
- **Compiler**: GCC 11.4.0 (`g++ -std=c++20`)
- **Build System**: CMake 3.22.1 & Ninja 1.10.1
- **Hardware Architecture**: x86_64
- **CompileForge Target Revision**: Commit `main`

---

## 2. Reproduction Steps

### A. Compile CompileForge
```bash
git clone https://github.com/Destroyer795/compileforge.git
cd compileforge
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### B. Execute Unit & Invariant Test Suite
```bash
./build/compileforge_tests
```

### C. Run Performance Benchmarks
```bash
./build/compileforge_benchmarks
```

### D. Run End-to-End Change-Impact Validation
```bash
# 1. Generate prediction for shared header modification
./build/compileforge impact ./examples/impact_demo_app HEAD~1..HEAD \
    --format json --output examples/impact_demo_app/prediction.json

# 2. Capture actual build execution into log
cd examples/impact_demo_app
g++ -Iinclude -std=c++20 -c src/network/client.cpp -o src/network/client.o > build.log 2>&1
g++ -Iinclude -std=c++20 -c src/storage/db.cpp -o src/storage/db.o >> build.log 2>&1
g++ -Iinclude -std=c++20 -c src/render/engine.cpp -o src/render/engine.o >> build.log 2>&1
cd ../..

# 3. Validate prediction against real build log
./build/compileforge validate examples/impact_demo_app/prediction.json \
    --log examples/impact_demo_app/build.log
```

---

## 3. Measured Raw Results

- **JSON Parser Throughput**: 195.95 MB/s (10.31 ms total for 20 iterations)
- **Preprocessor Lexer Speed**: 5,840,701 lines/sec (51.36 ms total)
- **260-File Multi-Tier Project Discovery & Scan**: 1,659.54 ms
- **Change Impact Precision**: 100.0%
- **Change Impact Recall**: 100.0%
- **Rebuild Surface Error Delta**: 0.0%

---

## 4. Evaluation Limitations & Scope

- CompileForge evaluates dependency blast radius via static preprocessor token analysis.
- Build cost predictions require compiler `-ftime-trace` logs to compute compilation durations; when absent, the tool transparently reports `UNAVAILABLE`.
