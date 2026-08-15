# CompileForge Data Flow Architecture

This document describes how data moves through CompileForge during whole-project analysis and change-impact workflows.

---

## 1. Change-Impact & Prediction Validation Sequence

The following sequence diagram details the end-to-end flow from Git revision diffing to observed build validation:

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

---

## 2. Project Analysis Data Flow (`compileforge analyze`)

When analyzing a full codebase:

1. **Discovery**: `ProjectScanner` traverses the directory tree, applying ignore rules from `.compileforge.json`.
2. **Compilation Flags**: `CompilationDatabase` parses compiler invocation flags from `compile_commands.json` (extracting search directories, `-std=`, `-O`, defines).
3. **Lexical Parsing**: `IncludeParser` extracts `#include` directives, skipping inactive `#if 0` code blocks.
4. **Graph Construction**: `DependencyGraph` registers files as nodes and includes as directed edges (`caller -> callee`).
5. **Topology Metrics**: Computes fan-in, fan-out, dependency depth, and Tarjan's SCC cycles.
6. **Health Scoring**: `BuildHealthScorer` evaluates flag consistency and structural health (0–100).
7. **Action Plan**: `RecommendationEngine` generates prioritized refactoring recommendations.
