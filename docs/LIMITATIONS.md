# Known Limitations & Technical Scope

CompileForge is designed to provide actionable build intelligence and change-impact analysis quickly. To maintain credibility with engineering teams and technical evaluators, this document explicitly outlines the technical boundaries and known limitations of the current implementation.

---

## 1. Static Lexical Analysis vs Semantic AST

- **No Semantic Type Resolution**: CompileForge parses `#include` directives, `#pragma once`, and `#ifndef` include guards via a high-speed preprocessor lexer (>5.3M lines/sec). It does not perform full C++ template instantiation or semantic AST analysis.
- **Conditional Compilation**: Include directives inside `#if 0` blocks are properly skipped. However, directives guarded by custom compile-time macro flags (e.g. `#ifdef USE_EXPERIMENTAL_FEATURE`) are parsed lexically as potential dependencies unless configured otherwise in `.compileforge.json`.

---

## 2. Build System & Caching Variance

- **Potential vs Observed Rebuilds**: CompileForge's "Potentially Affected Translation Units" represents the static header dependency blast radius. Modern build systems using tools such as `ccache`, `sccache`, or incremental linking with unchanged AST hashes may compile fewer translation units than statically predicted.
- **Precompiled Headers (PCH)**: PCH files aggregate multiple headers into a single binary artifact. Modifications to any header within a PCH will trigger recompilation of all translation units including that PCH, which may exceed standard include-graph blast radius estimates.

---

## 3. Build Cost & Timing Availability

- **Historical Timing Requirements**: Accurate build-cost prediction (in seconds) requires historical compilation data (such as Clang `-ftime-trace` profiles). When timing data is not present in the environment, CompileForge transparently reports `UNAVAILABLE` rather than fabricating synthetic estimates.

---

## 4. Evaluation Scope

- **Demonstration Scenario**: The 100% precision and recall metrics reported in documentation were measured on the controlled 3-TU demonstration scenario (`examples/impact_demo_app`). This result validates the algorithm on a controlled system but does not represent generalized statistical accuracy across arbitrary external codebases.
- **Real-World Case Studies**: The automated evaluation harness is provided in `evaluations/run_evaluation.py`. Dynamic evaluations on large external repositories have not yet been executed in this session.
