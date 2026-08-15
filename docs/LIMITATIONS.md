# Known Limitations & Technical Scope

CompileForge is designed to provide rapid build intelligence and change-impact analysis without external dependencies or slow semantic AST compilations. This document outlines the technical boundaries, known limitations, and operating assumptions of the tool.

---

## 1. Static Lexical Preprocessor vs Semantic AST

- **No Semantic Type Resolution**: CompileForge parses `#include` directives, `#pragma once`, and `#ifndef` include guards via a high-speed preprocessor lexer (>5.3M lines/sec). It does not perform full C++ template instantiation, constexpr evaluation, or semantic AST analysis.
- **Conditional Compilation**: Include directives inside `#if 0` blocks are skipped. However, directives guarded by custom compile-time macro flags (e.g. `#ifdef ENABLE_FEATURE_X`) are parsed lexically as potential dependencies unless configured otherwise in `.compileforge.json`.

---

## 2. Build System & Caching Variance

- **Potential vs Observed Rebuilds**: CompileForge's "Potentially Affected Translation Units" represents the static header dependency blast radius. Modern build systems using tools such as `ccache`, `sccache`, or incremental linking with unchanged AST hashes may compile fewer translation units than statically predicted.
- **Precompiled Headers (PCH)**: PCH files aggregate multiple headers into a single binary artifact. Modifications to any header within a PCH will trigger recompilation of all translation units including that PCH, which may exceed standard include-graph blast radius estimates.

---

## 3. Generated Files & Build-Time Code Generation

- Files generated dynamically during the build process (e.g. Protocol Buffers, Flex/Bison, Qt `moc`, CMake `configure_file`) will only appear in the dependency graph if they exist on disk prior to analysis or are referenced in `compile_commands.json`.

---

## 4. Build Cost & Timing Availability

- **Historical Timing Requirements**: Accurate build-cost prediction (in seconds) requires historical compilation data (such as Clang `-ftime-trace` profiles). When timing data is not present in the environment, CompileForge transparently reports `UNAVAILABLE` rather than fabricating synthetic estimates.

---

## 5. Prediction vs Guarantee

- CompileForge provides an **architectural prioritization heuristic**, not a mathematical proof or defect prediction guarantee.
- A high Change Risk Score indicates wide rebuild surface, deep dependency chains, or central header involvement. It does not indicate that a change contains bugs or syntax errors.

---

## 6. Evaluation Scope

- The 100% precision and recall metrics reported in documentation were measured on the controlled 3-TU demonstration scenario (`examples/impact_demo_app`). This result validates algorithmic correctness on that test fixture and is **not a claim of universal accuracy across arbitrary external codebases**.
- An automated evaluation harness is provided in `evaluations/run_evaluation.py` for evaluating external repositories.
