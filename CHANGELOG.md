# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-08-15

### Added
- **Core C++20 Engine**: Zero-runtime-dependency implementation using standard ISO C++20.
- **Git Change-Impact Analysis**: Trace Git revision diffs through `#include` dependency graphs to forecast affected translation units and rebuild surface.
- **Explainable Change Risk Scoring**: 0–100 risk heuristic combining blast radius, dependency depth, architecture centrality, Git churn, and preprocessor complexity.
- **Prediction Validation**: Closed-loop engine to compare predicted rebuild surface against actual compiler activity from build logs or live commands.
- **Whole-Project Health Analysis**: Graph topology analysis, Tarjan's SCC circular inclusion detection, compilation database ingestion, and 0–100 Build Health scoring.
- **Multi-Format Reporting**: Terminal summary cards with ANSI coloring, machine-readable JSON schemas (`1.0_impact`, `1.0_validation`), and standalone responsive HTML reports in Mid-Century Modern technical editorial design.
- **CI/CD Integration**: Native policy gating flags (`--fail-on-risk <N>`, `--fail-on-cycle`, `--fail-on-hotspot`) for pull request automation.
- **Automated Test Suite**: 41 unit, integration, and invariant tests with zero third-party dependencies.
