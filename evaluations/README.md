# CompileForge Real-World Evaluation Harness

This directory contains evaluation scripts to benchmark CompileForge against external open-source C++ repositories (e.g. `fmtlib/fmt`, `gabime/spdlog`, `catchorg/Catch2`).

## Reproducible Evaluation Workflow

```bash
# Evaluate an open-source C++ repository
python3 evaluations/run_evaluation.py --repo https://github.com/fmtlib/fmt.git --name fmt
```

## Third-Party Source Code Policy

CompileForge does **NOT** vendor or embed external third-party source code in its repository. External codebases are cloned dynamically during evaluation runs and ignored by Git.
