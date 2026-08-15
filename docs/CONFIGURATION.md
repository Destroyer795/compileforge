# CompileForge Configuration Schema (.compileforge.json)

CompileForge can be configured on a per-project basis using a `.compileforge.json` file in the root directory.

## Sample Configuration

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

## Schema Reference

| Property | Type | Default | Description |
| :--- | :--- | :--- | :--- |
| `ignore_patterns` | `array[string]` | `["build", ".git"]` | Directories or glob patterns to exclude from analysis |
| `fan_in_threshold` | `integer` | `20` | Transitive dependent count triggering high fan-in warnings |
| `fan_out_threshold` | `integer` | `15` | Include depth triggering high fan-out warnings |
| `hotspot_score_threshold` | `float` | `80.0` | Hotspot score threshold for CI `--fail-on-hotspot` |
| `complexity_threshold` | `integer` | `15` | Cyclomatic complexity threshold |
