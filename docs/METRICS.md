# CompileForge Metrics & Scoring Methodology

CompileForge utilizes static lexical analysis and dependency graph topology to calculate build intelligence metrics.

> [!NOTE]
> **Static Lexical Analysis Disclaimer**: CompileForge performs fast static lexical analysis of source files and compilation databases. It does NOT invoke a full compiler C++ AST or semantic preprocessor parser. All unused header predictions or complexity indices are lexical estimates.

## 1. Hotspot Score (0–100)

The Hotspot Score evaluates the compilation penalty of an individual header or translation unit:

$$\text{Hotspot Score} = \text{Build Cost} + \text{Fan-In Impact} + \text{Complexity} + \text{Commit Churn}$$

- **Build Cost (35%)**: Normalized preprocessed LOC size relative to the largest project file.
- **Fan-In Impact (30%)**: Transitive dependent count (how many translation units rely on this header).
- **Cyclomatic Complexity (20%)**: Control statement density (`if`, `while`, `for`, `switch`, `case`, `catch`).
- **Commit Churn (15%)**: Git commit modification frequency over project history.

## 2. Build Health Score (0–100)

The Build Health Score rates overall repository build configuration quality:

- **Base Score**: 100
- **Deductions**:
  - **-15 points** per circular dependency loop (`A.hpp -> B.hpp -> C.hpp -> A.hpp`).
  - **-10 points** per build configuration health finding (e.g., mixed `-O` flags or missing warning flags).
  - **-5 points** per high fan-in header (>20 transitive dependents).

## 3. Translation Unit Cost Model

Translation units are classified into cost tiers (`LOW`, `MEDIUM`, `HIGH`, `CRITICAL`) based on:

$$\text{Cost Score} = 0.2 \times \text{SLOC} + 5.0 \times \text{Transitive Headers} + 2.0 \times \text{Macros} + 3.0 \times \text{Templates}$$

- **LOW**: Score < 50
- **MEDIUM**: Score 50–149
- **HIGH**: Score 150–299
- **CRITICAL**: Score $\ge$ 300
