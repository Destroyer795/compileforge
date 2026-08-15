# CompileForge CLI Command Reference

This document details all supported CLI commands, arguments, options, output formats, and exit codes.

---

## 1. `compileforge analyze`

Performs whole-project discovery, compilation database ingestion, include graph construction, and health scoring.

### Syntax
```bash
compileforge analyze [project_dir] [options]
```

### Options
| Option | Description | Default |
| :--- | :--- | :--- |
| `--db <path>` | Explicit path to `compile_commands.json` | Auto-discovered in project root / `build/` |
| `--format <type>` | Output format: `terminal`, `json`, or `html` | `terminal` |
| `--output <file>` | Output filepath for JSON or HTML report | Stdout for terminal |
| `--fail-on-cycle` | Exit with status 1 if any circular dependency loop is detected | Disabled |
| `--fail-on-hotspot` | Exit with status 1 if any critical architectural hotspot is identified | Disabled |
| `--no-color` | Disable ANSI color sequences in terminal output | Color enabled |

### Example
```bash
compileforge analyze . --format html --output build/health_report.html --fail-on-cycle
```

---

## 2. `compileforge impact`

Analyzes a Git revision diff or uncommitted working tree changes to calculate the potentially affected rebuild surface and change risk score.

### Syntax
```bash
compileforge impact [project_dir] [git_range] [options]
```

### Arguments
- `[project_dir]`: Target project root directory (default: `.`).
- `[git_range]`: Git revision range (e.g. `HEAD~1..HEAD`, `origin/main..HEAD`, or omitted for working tree diff).

### Options
| Option | Description | Default |
| :--- | :--- | :--- |
| `--format <type>` | Output format: `terminal`, `json`, or `html` | `terminal` |
| `--output <file>` | Output destination for JSON or HTML report | Stdout |
| `--fail-on-risk <N>` | Exit with status 1 if Change Risk Score $\ge N$ (0–100) | Disabled |

### Example
```bash
compileforge impact . HEAD~1..HEAD --format json --output prediction.json --fail-on-risk 70
```

---

## 3. `compileforge validate`

Validates impact predictions against actual compiler build logs or executes a build command to observe recompilation activity.

### Syntax
```bash
compileforge validate <prediction.json> [options]
```

### Options
| Option | Description | Default |
| :--- | :--- | :--- |
| `--log <file>` | Path to compiler build log (Ninja, Make, GCC, Clang) | None |
| `--build "<cmd>"` | Command line to execute and observe (e.g. `"ninja -C build"`) | None |
| `--format <type>` | Output format: `terminal`, `json`, or `html` | `terminal` |
| `--output <file>` | Output destination for JSON or HTML validation report | Stdout |

### Example
```bash
compileforge validate prediction.json --log build.log --format html --output validation_report.html
```

---

## 4. Helper Commands

### `compileforge diff <baseline.json> <target.json>`
Compares two project analysis JSON reports and displays metric deltas (files, edges, health score, cycles).

### `compileforge diff-impact <baseline_impact.json> <current_impact.json>`
Compares two impact analysis JSON reports and displays risk and rebuild surface deltas.

### `compileforge init`
Generates a default `.compileforge.json` configuration file in the current directory.

---

## 5. Exit Codes

| Exit Code | Meaning |
| :--- | :--- |
| `0` | Success (Analysis completed, risk within acceptable threshold). |
| `1` | Error or Policy Failure (Risk threshold exceeded, circular dependency found with `--fail-on-cycle`, missing files, or invalid arguments). |
