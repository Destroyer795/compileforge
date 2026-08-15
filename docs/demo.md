# CompileForge Product Walkthrough & Demo Guide

This guide demonstrates the full **DETECT → PREDICT → BUILD → VERIFY → LEARN** workflow using CompileForge's included 3-TU demonstration fixture.

---

### Step 1: Make a Code Change to a Shared Header (DETECT)

In the demo project (`examples/impact_demo_app`), modify a core shared header:

```bash
cd examples/impact_demo_app
# Modify include/core/types.hpp
git commit -am "feat(core): update Message struct in types.hpp"
```

---

### Step 2: Predict Change Blast Radius & Build Risk (PREDICT)

Run CompileForge Change-Impact analysis before merging or rebuilding:

```bash
compileforge impact HEAD~1..HEAD --format json --output prediction.json
```

**Output:**
```
==========================================================
             COMPILEFORGE CHANGE-IMPACT ANALYSIS          
==========================================================

CHANGE RISK SCORE: 38 / 100 (CRITICAL IMPACT)

CHANGE SUMMARY
  Changed Files:     1
  Potentially Affected TUs: 3
  Potentially Affected Headers: 2
  Estimated Rebuild Surface: 100.0%
  Max Impact Depth:  1 levels

WHY THIS CHANGE IS RISKY
  - affects 3 translation unit(s) (100.0% of project)

REVIEW HOTSPOTS (HIGH INSPECTION PRIORITY)
  include/core/types.hpp
```

---

### Step 3: Execute the Project Build (BUILD)

Execute your standard build tool or compiler (e.g. GCC or Ninja):

```bash
g++ -Iinclude -std=c++20 -c src/network/client.cpp -o src/network/client.o > build.log 2>&1
g++ -Iinclude -std=c++20 -c src/storage/db.cpp -o src/storage/db.o >> build.log 2>&1
g++ -Iinclude -std=c++20 -c src/render/engine.cpp -o src/render/engine.o >> build.log 2>&1
```

---

### Step 4: Validate Predictions Against Observed Activity (VERIFY)

Compare what was predicted against what the compiler actually rebuilt:

```bash
compileforge validate prediction.json --log build.log
```

**Validation Output:**
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

*(Note: The included demonstration scenario achieved 100% precision and recall on this 3-TU system.)*
