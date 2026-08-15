# CompileForge Reporting & Output Formats

CompileForge generates three complementary report formats designed for developers, CI systems, and code reviewers.

---

## 1. Output Format Comparison

| Format | Flag | Primary Audience | Key Characteristics |
| :--- | :--- | :--- | :--- |
| **Terminal** | `--format terminal` | Developer / CLI | ANSI-colored visual summary cards with tables and risk indicators. |
| **JSON** | `--format json` | CI / Tooling | Structured, machine-readable JSON payloads with explicit schema versions. |
| **HTML** | `--format html` | Code Reviewers / CI Artifacts | Self-contained, responsive dashboard with zero external CDN dependencies. |

---

## 2. Machine-Readable JSON Schemas

### Impact Report Schema (`schema_version: "1.0_impact"`)
Includes:
- `risk`: Total risk score (0–100) and multi-factor breakdown (impact, build cost, architecture, churn, complexity, cycle factors).
- `rebuild_surface`: Total affected translation units, headers, affected files, % of project TUs, max impact depth, and `affected_translation_units` list.
- `changed_files`: List of modified, added, renamed, or deleted files.
- `review_hotspots`: Top files recommended for close review with specific reasons.
- `why_risky`: Human-readable bullet explanations.

### Validation Report Schema (`schema_version: "1.0_validation"`)
Includes:
- `accuracy`: True positives, false positives, false negatives, precision %, recall %, rebuild surface error delta %, and overall accuracy rating (`EXCELLENT`, `GOOD`, `MODERATE`, `LOW`).
- `observed`: List of observed rebuilt translation units and measured build duration.
- `prediction`: Copy of the original predicted metrics.

---

## 3. Interactive HTML Dashboard

CompileForge HTML reports are completely self-contained. All styles, typography, and SVG metric meters are embedded directly in a single `.html` file, ensuring they render correctly when downloaded as CI pipeline artifacts or viewed offline.
