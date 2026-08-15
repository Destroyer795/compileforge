# CompileForge Continuous Integration Guide

CompileForge provides deterministic exit codes and machine-readable JSON outputs for seamless integration into GitHub Actions, GitLab CI, and Jenkins.

## Exit Codes

- `0`: Analysis successful, no architectural regressions or policy failures.
- `1`: CI policy failure (e.g. `--fail-on-cycle`, `--fail-on-hotspot`, or regression detected).

## Command Line Flags for CI

```bash
# Fail CI build if any circular dependency loop is introduced
compileforge analyze . --fail-on-cycle

# Fail CI build if any file exceeds hotspot score 80.0
compileforge analyze . --fail-on-hotspot --format json --output report.json

# Fail CI build if build health regresses compared to baseline report
compileforge diff baseline.json report.json
```
