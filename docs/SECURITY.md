# Security, Provenance & Threat Model

This document outlines the security architecture, data handling practices, and dependency provenance of CompileForge.

---

## 1. Zero Third-Party Runtime Dependencies

CompileForge is implemented entirely using standard **ISO C++20** and the Standard Template Library (STL).
- **No Third-Party Runtime Libraries**: No external JSON parser, regex engine, or graph library is linked into the binary.
- **Supply Chain Safety**: Zero risk of transitive dependency vulnerabilities or upstream supply-chain compromises.

---

## 2. Data Privacy & Confidentiality

- **Zero Telemetry / Network Calls**: CompileForge runs entirely offline and performs no outbound network requests or analytics tracking.
- **Zero Tracked Credentials**: No API keys, credentials, or personal workstation paths are stored or transmitted.
- **Read-Only Project Access**: During `analyze` and `impact` operations, CompileForge performs only read operations on project source files. Output reports are written exclusively to explicitly requested paths or standard output.

---

## 3. Threat Model & Process Invocation

- **Git Invocation**: Git subprocess commands (`git diff`, `git rev-parse`) are executed using sanitized arguments.
- **Build Observation**: In `compileforge validate --build "<cmd>"`, CompileForge executes user-provided build commands in the target project working directory. This command should only be invoked in trusted build environments.
- **Parser Robustness**: The JSON and Preprocessor parsers are fuzz-tested against malformed tokens, infinite include recursions, and unexpected control characters.
