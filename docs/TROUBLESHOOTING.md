# CompileForge Troubleshooting Guide

This guide provides diagnostics and remediation steps for common issues encountered during analysis, change-impact prediction, or validation.

---

## 1. Missing Compilation Database (`compile_commands.json`)

### Diagnostic
```
[ERROR] Compilation database not found in project root or build/
```

### Remediation
Ensure your build system is configured to export compilation commands:
- **CMake**: Re-configure with `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`:
  ```bash
  cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
  ```
- **Ninja/Make**: Ensure `compile_commands.json` is located in the project root or `build/` directory, or specify its path explicitly via `--db`:
  ```bash
  compileforge analyze . --db path/to/compile_commands.json
  ```

---

## 2. Git Revision Range Issues

### Diagnostic
```
[ERROR] Failed to execute git diff for range: origin/main..HEAD
```

### Remediation
- Verify that the target Git repository is initialized and has commits:
  ```bash
  git log -n 1
  ```
- If comparing against a remote branch, ensure the remote reference exists locally:
  ```bash
  git fetch origin main
  ```
- If analyzing uncommitted changes in the current working directory, run `compileforge impact` without specifying a revision range.

---

## 3. Build Log Format Parsing

### Diagnostic
```
Observed Rebuilt TUs: 0 [OBSERVED] (Recall: UNAVAILABLE)
```

### Remediation
- Ensure the build log contains standard compiler invocation lines (e.g. `g++ -c`, `clang++ -c`, or Ninja `[X/Y] Building CXX object ... .dir/path.cpp.o`).
- Capture verbose compiler output when using custom build scripts:
  ```bash
  ninja -v > build.log 2>&1
  compileforge validate prediction.json --log build.log
  ```

---

## 4. Cross-Platform Path Handling

CompileForge internally normalizes all file paths using forward slashes (`/`). On Windows, drive letters (e.g. `C:/`) and relative directory separators (`\`) are normalized automatically.
