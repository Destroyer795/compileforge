# CompileForge Change-Impact Validation Case Study

**Target Project**: `examples/impact_demo_app` (C++20 Multi-Module App)  
**Evaluation Type**: Case Study — One Controlled System

---

## 1. Project Overview

`impact_demo_app` represents a modular C++20 architecture with:
- A central shared data contract: `include/core/types.hpp`
- A network module: `include/network/client.hpp` and `src/network/client.cpp`
- A storage engine: `src/storage/db.cpp`
- A rendering subsystem: `src/render/engine.cpp`
- A total of 3 translation units and 2 headers.

---

## 2. Test Change

A Git modification was introduced to the shared data contract in `include/core/types.hpp` (`feat(core): modify shared types header to trigger change-impact`).

```diff
--- a/include/core/types.hpp
+++ b/include/core/types.hpp
@@ -6,4 +6,5 @@ namespace demo {
         int id{0};
         std::string payload;
+        // modified shared header
     };
 }
```

---

## 3. Impact Prediction

CompileForge analyzed the Git revision range (`HEAD~1..HEAD`) before rebuilding:

- **Predicted Affected Translation Units**: `src/network/client.cpp`, `src/storage/db.cpp`, `src/render/engine.cpp` (3 TUs)
- **Predicted Rebuild Surface**: 100.0% of project translation units
- **Predicted Max Impact Depth**: 2 dependency levels
- **Change Risk Score**: 41 / 100 (CRITICAL IMPACT)

---

## 4. Observed Build

The project translation units were compiled using GCC 11.4 (`g++ -std=c++20`), producing an actual compiler build log with 3 compiler invocations.

- **Observed Rebuilt Translation Units**: 3 TUs (`src/network/client.cpp`, `src/storage/db.cpp`, `src/render/engine.cpp`)

---

## 5. Prediction Accuracy & Metrics

```
==========================================================
             CHANGE IMPACT PREDICTION VALIDATION          
==========================================================

PREDICTION VS OBSERVATION
  Predicted Affected TUs:      3 [ESTIMATED]
  Observed Rebuilt TUs:        3 [OBSERVED]
  True Positives (Correct):    3
  False Positives (Over-pred): 0
  False Negatives (Missed):    0

ACCURACY METRICS
  Prediction Precision:        100.0%
  Prediction Recall:           100.0%
  Rebuild Surface Error Delta: 0.0%
  Build-Cost Error Delta:      UNAVAILABLE (No historical build timings)
  Overall Accuracy Rating:     EXCELLENT
==========================================================
```

---

## 6. What CompileForge Got Wrong & Limitations

1. **Compiler Caching Unawareness**: CompileForge predicts build surface based on pure static `#include` topology. If a project utilizes `ccache`, `sccache`, or incremental linking with unchanged AST hashes, the build system may skip recompilation of translation units that CompileForge flags as affected.
2. **Conditional Header Inclusion Outside `#if 0`**: If `#include` directives are guarded by custom preprocessor macros that are disabled in the target build flags (e.g. `#ifdef ENABLE_EXPERIMENTAL_FEATURE`), CompileForge's lexical analyzer still identifies the dependency unless defined in `.compileforge.json`.
3. **Historical Timing Absence**: In the absence of `-ftime-trace` logs, CompileForge cannot infer compilation durations and outputs `UNAVAILABLE` rather than estimating seconds.

---

## 7. Conclusion

In this evaluation, CompileForge correctly identified all 3 affected translation units with 100.0% precision and recall, accurately forecasting a 100% rebuild surface before the build was executed.
