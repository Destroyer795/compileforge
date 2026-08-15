# CompileForge Architecture & Data Flow

CompileForge is a native C++20, zero-runtime-dependency build intelligence and change-impact analysis toolkit. It bridges Git revision history, preprocessor inclusion graphs, compilation databases, and compiler build logs.

---

## 1. Design Goals

1. **Native C++20 Implementation**: Leverages modern standard library facilities (concepts, `std::filesystem`, `std::string_view`, `std::optional`, `std::variant`).
2. **Zero Runtime Dependencies**: Handcrafted JSON parser, preprocessor lexer, and graph algorithms without external dependencies (no Boost, no third-party JSON or graph libraries).
3. **Deterministic & Explainable**: Identical inputs produce identical graph traversals, risk scores, and recommendations. All risk and health scores are backed by explicit data-driven reason arrays.
4. **Closed-Loop Workflow**: Connects change prediction (`compileforge impact`) with actual build observation and validation (`compileforge validate`).
5. **CI Automation Ready**: Native policy thresholds (`--fail-on-risk <N>`, `--fail-on-cycle`), predictable exit codes, and machine-readable JSON outputs.

---

## 2. System Architecture

```mermaid
flowchart TB
    subgraph CLI Layer
        CLI["CLI Entrypoint (main.cpp)"]
    end

    subgraph Ingestion & Discovery
        Scanner["ProjectScanner\n(src/project)"]
        CompDb["CompilationDatabase\n(src/parser)"]
        Parser["IncludeParser\n(src/parser)"]
        GitAn["GitAnalyzer\n(src/git)"]
    end

    subgraph Core Graph & Topology
        Graph["DependencyGraph\n(src/graph)"]
        CycleDet["CycleDetector (Tarjan SCC)\n(src/graph)"]
        Metrics["SourceMetrics\n(src/metrics)"]
    end

    subgraph Intelligence & Scoring
        Hotspot["HotspotScorer\n(src/analysis)"]
        TUCost["TUCostAnalyzer\n(src/analysis)"]
        HealthScore["BuildHealthScore\n(src/analysis)"]
        Impact["ImpactAnalyzer (BFS)\n(src/impact)"]
        Risk["RiskScorer (0-100)\n(src/impact)"]
        Recom["RecommendationEngine\n(src/recommendations)"]
    end

    subgraph Observation & Validation
        Obs["BuildObserver\n(src/validation)"]
        Val["ImpactValidator\n(src/validation)"]
    end

    subgraph Reporting Layer
        RepTerm["TerminalReporter\n(src/reporting)"]
        RepJSON["JsonReporter\n(src/reporting)"]
        RepHTML["HtmlReporter / ImpactHtmlReporter\n(src/reporting)"]
        RepVal["ValidationReporter\n(src/reporting)"]
    end

    CLI --> Scanner
    CLI --> CompDb
    CLI --> Parser
    CLI --> GitAn
    CLI --> Obs

    Scanner --> Graph
    Parser --> Graph
    CompDb --> Graph
    Graph --> CycleDet
    Graph --> Metrics

    Graph --> Hotspot
    Graph --> TUCost
    Hotspot --> HealthScore
    TUCost --> HealthScore

    GitAn --> Impact
    Graph --> Impact
    Impact --> Risk
    HealthScore --> Recom

    Impact --> Val
    Obs --> Val

    HealthScore --> RepTerm
    HealthScore --> RepJSON
    HealthScore --> RepHTML
    Risk --> RepJSON
    Risk --> RepHTML
    Val --> RepVal
```

---

## 3. Subsystem Breakdown

### 1. Ingestion Layer (`compileforge::parser`, `compileforge::git`, `compileforge::project`)
- `ProjectScanner`: Recursively traverses directory trees, recognizing C++ sources and headers while filtering ignore patterns from `.compileforge.json`.
- `CompilationDatabase`: Ingests standard Clang `compile_commands.json` files, extracting compiler binary paths, language standards (`-std=c++20`), include search paths (`-I`), preprocessor definitions (`-D`), and optimization levels.
- `IncludeParser`: High-throughput preprocessor lexer (>5.3M lines/sec) extracting `#include` directives, `#pragma once`, and `#ifndef` guards, while bypassing inactive `#if 0` code sections.
- `GitAnalyzer`: Parses Git revision diffs (`git diff --name-status -M`) to identify modified, added, renamed, or deleted files.

### 2. Graph & Topology Layer (`compileforge::graph`, `compileforge::metrics`)
- `DependencyGraph`: Directed graph modeling preprocessor dependencies (`caller -> callee`). Computes direct and transitive fan-in (centrality) and fan-out (breadth).
- `CycleDetector`: Computes Strongly Connected Components via Tarjan's algorithm in $O(V + E)$ linear time to detect circular inclusion loops.
- `SourceMetrics`: Computes line counts (SLOC, blank, comment), preprocessor density, and cyclomatic complexity.

### 3. Change-Impact & Risk Engine (`compileforge::impact`, `compileforge::analysis`)
- `ImpactAnalyzer`: Propagates changes outward along incoming dependent edges using breadth-first search (BFS) to compute the set of affected translation units, maximum dependency depth, and rebuild surface percentage.
- `RiskScorer`: Evaluates an explainable 0–100 Change Risk Score across six factors (Impact, Depth/Cost, Architecture/Centrality, Churn, Complexity, Cycles) and generates data-backed justification points.
- `HotspotScorer`: Evaluates compilation cost, transitive fan-in, complexity, and commit frequency.
- `BuildHealthScore`: Rates overall repository build configuration quality on a 0–100 scale.
- `RecommendationEngine`: Generates prioritized, actionable refactoring steps.

### 4. Build Observation & Validation Engine (`compileforge::validation`)
- `BuildObserver`: Captures and parses compiler build activity from live build commands or Ninja/Make/GCC/Clang build logs.
- `ImpactValidator`: Calculates statistical accuracy metrics ($TP, FP, FN$, precision, recall, rebuild surface error delta) to validate predictions against actual compiler builds.

### 5. Reporting Layer (`compileforge::reporting`)
- `TerminalReporter`: ANSI-colored visual summary cards with tables and indicators.
- `JsonReporter`: Machine-readable payloads conforming to versioned schemas (`1.0_impact`, `1.0_validation`).
- `HtmlReporter` / `ImpactHtmlReporter` / `ValidationReporter`: Standalone single-file HTML reports using a Mid-Century Modern Technical Editorial visual design.

---

## 4. End-to-End Data Flow

### Change-Impact & Prediction Validation Sequence

```mermaid
sequenceDiagram
    autonumber
    actor Dev as Developer / CI
    participant CLI as CompileForge CLI
    participant Git as Git Analyzer
    participant Scanner as Project Scanner
    participant Parser as Include / DB Parser
    participant Graph as Dependency Graph
    participant Impact as Impact Analyzer
    participant Risk as Risk Scorer
    participant Observer as Build Observer
    participant Val as Impact Validator
    participant Rep as Reporter

    %% Phase 1: Prediction
    Dev->>CLI: compileforge impact HEAD~1..HEAD --format json --output pred.json
    CLI->>Git: get_changed_files("HEAD~1..HEAD")
    Git-->>CLI: ChangedFileEntry[] (added, modified, renamed, deleted)
    CLI->>Scanner: scan(project_root)
    Scanner-->>CLI: FileNode[]
    CLI->>Parser: parse compile_commands.json & #include directives
    Parser->>Graph: build nodes & forward/backward edges
    Graph-->>CLI: Graph Ready (Fan-In/Out, SCC Cycles)
    CLI->>Impact: analyze_impact(Graph, ChangedFileEntry[])
    Impact-->>CLI: ImpactAnalysisResult (affected TUs, rebuild surface %, depth)
    CLI->>Risk: compute_risk(Impact, Graph, cycle_count)
    Risk-->>CLI: ChangeRiskResult (0-100 score, "Why Risky" bullets)
    CLI->>Rep: render(Terminal / JSON / HTML)
    Rep-->>Dev: Prediction Report (saved to pred.json)

    %% Phase 2: Build & Validation
    Dev->>CLI: compileforge validate pred.json --log build.log
    CLI->>Observer: parse_build_log("build.log")
    Observer-->>CLI: BuildObservation (actually rebuilt TUs, duration)
    CLI->>Val: validate(ImpactPrediction, BuildObservation)
    Val-->>CLI: ImpactValidationResult (precision, recall, error deltas)
    CLI->>Rep: render_validation(Terminal / JSON / HTML)
    Rep-->>Dev: Accuracy & Verification Summary
```

### Whole-Project Analysis Flow (`compileforge analyze`)

1. **Discovery**: `ProjectScanner` traverses the directory tree, applying ignore rules from `.compileforge.json`.
2. **Compilation Flags**: `CompilationDatabase` parses compiler invocation flags from `compile_commands.json` (extracting search directories, `-std=`, `-O`, defines).
3. **Lexical Parsing**: `IncludeParser` extracts `#include` directives, skipping inactive `#if 0` code blocks.
4. **Graph Construction**: `DependencyGraph` registers files as nodes and includes as directed edges (`caller -> callee`).
5. **Topology Metrics**: Computes fan-in, fan-out, dependency depth, and Tarjan's SCC cycles.
6. **Health Scoring**: `BuildHealthScorer` evaluates flag consistency and structural health (0–100).
7. **Action Plan**: `RecommendationEngine` generates prioritized refactoring recommendations.
