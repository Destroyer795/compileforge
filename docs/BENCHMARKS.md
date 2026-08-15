# CompileForge Performance & Benchmarks

This document records the measured performance metrics of CompileForge's core parsing and analysis engines.

---

## 1. Measured Benchmark Results

> [!NOTE]
> All benchmarks were executed on the primary development test environment:
> **Platform**: Ubuntu 22.04 LTS via WSL2 on Windows 11 (x86_64, GCC 11.4.0, Release build with `-O3`).
> These numbers represent measured timings on this test environment and are not universal performance guarantees.

| Benchmark Subsystem | Measured Throughput / Latency | Test Scope & Methodology |
| :--- | :--- | :--- |
| **JSON Parser Engine** | **187.15 MB/s** | Iterative ingestion of synthetic compilation database payloads (20 iterations, 10.80 ms total). |
| **Preprocessor Lexer** | **5,385,355 lines/sec** | High-throughput lexical scanning of `#include`, include guards, and `#if 0` blocks (55.71 ms total). |
| **Project Scan & Graph Construction** | **1,464.62 ms** | End-to-end directory traversal, compilation database parsing, and include graph construction for 260 C++ source and header files. |

---

## 2. Benchmark Methodology

CompileForge includes a standalone benchmark runner compiled into `compileforge_benchmarks`:

```bash
# Execute performance benchmark suite
./build/compileforge_benchmarks
```

The benchmark binary uses `std::chrono::high_resolution_clock` to record wall-clock elapsed time across isolated processing loops, verifying that processing throughput meets real-time CI latency requirements.
