# CompileForge CLI Reference, Configuration & Reporting

This document details all supported CLI commands, configuration options (`.compileforge.json`), output formats, JSON schemas, and exit codes.

---

## 1. CLI Commands

### `compileforge analyze`
Performs whole-project discovery, compilation database ingestion, include graph construction, and health scoring.

```bash
compileforge analyze [project_dir] [options]
```

| Option | Description | Default |
| :--- | :--- | :--- |
| `--db <path>` | Path to `compile_commands.json` | Auto-discovered in project root / `build/` |
| `--format <type>` | Output format: `terminal`, `json`, or `html` | `terminal` |
| `--output <file>` | Destination file for JSON or HTML report | Stdout |
| `--fail-on-cycle` | Exit with status 1 if any circular include loop is detected | Disabled |
| `--fail-on-hotspot` | Exit with status 1 if any critical architectural hotspot is identified | Disabled |
| `--no-color` | Disable ANSI color formatting in terminal output | Color enabled |

---

### `compileforge impact`
Analyzes a Git revision diff or uncommitted working tree changes to calculate the potentially affected rebuild surface and change risk score.

```bash
compileforge impact [project_dir] [git_range] [options]
```

| Argument / Option | Description | Default |
| :--- | :--- | :--- |
| `[project_dir]` | Target project root directory | `.` |
| `[git_range]` | Git revision range (e.g. `origin/main..HEAD` or `HEAD~1..HEAD`) | Uncommitted changes |
| `--format <type>` | Output format: `terminal`, `json`, or `html` | `terminal` |
| `--output <file>` | Destination file for JSON or HTML report | Stdout |
| `--fail-on-risk <N>` | Exit with status 1 if Change Risk Score $\ge N$ (0–100) | Disabled |

---

### `compileforge validate`
Validates impact predictions against actual compiler build logs or executes a build command to observe recompilation activity.

```bash
compileforge validate <prediction.json> [options]
```

| Option | Description | Default |
| :--- | :--- | :--- |
| `--log <file>` | Path to compiler build log (Ninja, Make, GCC, Clang) | None |
| `--build "<cmd>"` | Command string to execute and observe (e.g. `"ninja -C build"`) | None |
| `--format <type>` | Output format: `terminal`, `json`, or `html` | `terminal` |
| `--output <file>` | Destination file for JSON or HTML report | Stdout |

---

### Helper Commands

- `compileforge diff <baseline.json> <target.json>`: Compares two project analysis JSON reports and displays metric deltas (files, edges, health score, cycles).
- `compileforge diff-impact <baseline_impact.json> <current_impact.json>`: Compares two impact analysis JSON reports and displays risk and rebuild surface deltas.
- `compileforge init`: Generates a default `.compileforge.json` configuration file in the current directory.

---

## 2. Exit Codes

| Exit Code | Meaning |
| :--- | :--- |
| `0` | **Success**: Analysis completed cleanly, risk within acceptable threshold. |
| `1` | **Policy Failure or Error**: Risk threshold exceeded (`--fail-on-risk`), circular dependency found (`--fail-on-cycle`), missing files, or invalid arguments. |

---

## 3. Configuration Schema (`.compileforge.json`)

CompileForge can be configured on a per-project basis using a `.compileforge.json` file in the project root:

```json
{
  "ignore_patterns": [
    "build",
    "cmake-build-*",
    ".git",
    "out",
    "target",
    "node_modules"
  ],
  "fan_in_threshold": 20,
  "fan_out_threshold": 15,
  "complexity_threshold": 15,
  "dependency_depth_threshold": 10,
  "hotspot_score_threshold": 80.0,
  "churn_threshold": 10,
  "build_duration_threshold_seconds": 5.0,
  "include_count_threshold": 25
}
```

### Schema Properties

| Property | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `ignore_patterns` | `array[string]` | `["build", ".git"]` | Directories or glob patterns to exclude from scanning |
| `fan_in_threshold` | `integer` | `20` | Transitive dependent count triggering high fan-in warnings |
| `fan_out_threshold` | `integer` | `15` | Include count triggering high fan-out warnings |
| `hotspot_score_threshold` | `float` | `80.0` | Hotspot score threshold for CI `--fail-on-hotspot` |
| `complexity_threshold` | `integer` | `15` | Cyclomatic complexity threshold |
| `dependency_depth_threshold` | `integer` | `10` | Traversal depth threshold for deep inclusion chains |

---

## 4. Reporting & Output Formats

| Format | Flag | Audience | Description |
| :--- | :--- | :--- | :--- |
| **Terminal** | `--format terminal` | Developer / CLI | ANSI-colored visual summary cards with tables and indicators. |
| **JSON** | `--format json` | CI / Tooling | Structured, machine-readable JSON payloads with explicit schema versions. |
| **HTML** | `--format html` | Code Reviewers / CI | Standalone, self-contained Mid-Century Modern editorial dashboard with zero external CDN dependencies. |

### Machine-Readable JSON Schemas

#### Impact Report Schema (`schema_version: "1.0_impact"`)
```json
{
  "schema_version": "1.0_impact",
  "project_dir": ".",
  "git_range": "origin/main..HEAD",
  "risk": {
    "score": 38.0,
    "tier": "MODERATE",
    "factors": { "impact": 35.0, "depth_cost": 20.0, "architecture": 15.0, "churn": 10.0, "complexity": 10.0, "cycles": 0.0 }
  },
  "rebuild_surface": {
    "affected_translation_units_count": 3,
    "affected_headers_count": 2,
    "affected_translation_units": ["src/network/client.cpp", "src/storage/db.cpp", "src/render/engine.cpp"],
    "rebuild_surface_pct": 100.0,
    "max_depth": 2
  },
  "changed_files": [{ "path": "include/core/types.hpp", "status": "M" }],
  "review_hotspots": [{ "path": "include/core/types.hpp", "reasons": ["high transitive fan-in"] }],
  "why_risky": ["affects 3 translation unit(s) (100.0% of project)"]
}
```

#### Validation Report Schema (`schema_version: "1.0_validation"`)
```json
{
  "schema_version": "1.0_validation",
  "accuracy": {
    "true_positives": 3,
    "false_positives": 0,
    "false_negatives": 0,
    "precision_pct": 100.0,
    "recall_pct": 100.0,
    "rebuild_surface_error_delta_pct": 0.0,
    "overall_rating": "EXCELLENT"
  },
  "observation": {
    "observed_rebuilt_tus": ["src/network/client.cpp", "src/storage/db.cpp", "src/render/engine.cpp"]
  },
  "prediction": { "predicted_affected_tus_count": 3 }
}
```

---

## 5. HTML Visual Design System

CompileForge HTML reports use a **Mid-Century Modern × Technical Editorial** design language that emphasizes technical precision, readability, restraint, and craftsmanship.

- **Parchment Canvas**: Warm ivory/parchment background (`#f7f4ed`) and clean surface panels (`#fdfbf7`).
- **Restrained Palette**: Deep charcoal ink (`#24211e`), slate walnut muted text (`#6b635b`), olive green for verified/healthy states (`#4d6b42`), mustard for warnings (`#b0802c`), burnt orange for high risk (`#b84c24`), deep crimson for critical items (`#82261e`), and muted teal for metadata accents (`#2e6065`).
- **Typography Stack**: Humanist serif headlines (`"Newsreader", "Georgia", "Baskerville", serif`), clean system sans-serif body text, and monospace technical data (`"SF Mono", "Consolas", "Menlo", monospace`).
- **Single-File Zero CDN**: All styles, typography rules, and responsive layouts are embedded directly into a single `.html` file. Requires zero external network access, web fonts, or CDN dependencies.
