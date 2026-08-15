# CompileForge Reporting & Visual Design System

CompileForge generates three complementary report formats designed for developers, CI systems, and code reviewers.

---

## 1. Visual Design Philosophy: Mid-Century Modern Technical Editorial

CompileForge HTML reports use a **Mid-Century Modern × Technical Editorial** design language that emphasizes technical precision, readability, restraint, and craftsmanship rather than generic SaaS/AI dashboard tropes.

### Key Visual Tokens
- **Parchment Canvas**: Warm ivory/parchment background (`#f7f4ed`) and clean surface panels (`#fdfbf7`).
- **Restrained Palette**: Deep charcoal ink (`#24211e`), slate walnut muted text (`#6b635b`), olive green for verified/healthy states (`#4d6b42`), mustard for warnings (`#b0802c`), burnt orange for high risk (`#b84c24`), deep crimson for critical items (`#82261e`), and muted teal for metadata accents (`#2e6065`).
- **Editorial Typography Stack**: Humanist serif headlines (`"Newsreader", "Georgia", "Baskerville", serif`), clean system sans-serif body text, and monospace technical data (`"SF Mono", "Consolas", "Menlo", monospace`).
- **Typographic Metric Strip**: Clean horizontal strip with hairline vertical dividers (`1px solid #dfd8cb`), bold serif numbers, and uppercase monospace tracking labels.
- **Horizontal Risk Calibration**: Horizontal risk gauge with calibrated intervals (`LOW`, `MODERATE`, `HIGH`, `CRITICAL`) and indicator pins.
- **Two-Column Comparative Validation**: Two-column layout contrasting predicted blast radius with observed compiler activity.

---

## 2. Output Format Comparison

| Format | Flag | Primary Audience | Key Characteristics |
| :--- | :--- | :--- | :--- |
| **Terminal** | `--format terminal` | Developer / CLI | ANSI-colored visual summary cards with tables and risk indicators. |
| **JSON** | `--format json` | CI / Tooling | Structured, machine-readable JSON payloads with explicit schema versions. |
| **HTML** | `--format html` | Code Reviewers / CI Artifacts | Self-contained, responsive Mid-Century Modern editorial dashboard with zero external CDN dependencies. |

---

## 3. Machine-Readable JSON Schemas

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
- `observation`: List of observed rebuilt translation units and measured build duration.
- `prediction`: Copy of the original predicted metrics.

---

## 4. Single-File Self-Contained Architecture

CompileForge HTML reports are completely self-contained. All styles, typography rules, responsive breakpoints, and SVG elements are embedded directly in a single `.html` file. They require zero CDN scripts, external stylesheets, or web fonts, ensuring they function offline and archive cleanly as CI build artifacts.
