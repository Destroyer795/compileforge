# CompileForge Architecture & Design Specification

CompileForge is built as a zero-dependency, cross-platform C++20 build intelligence and optimization toolkit.

## System Architecture Overview

```
                      +-----------------------------+
                      |   CompileForge CLI Binary   |
                      +--------------+--------------+
                                     |
    +--------------------------------+--------------------------------+
    |                                |                                |
+---v------------------+  +----------v-----------+  +-----------------v----+
|   Project Scanner    |  | Compilation Database |  |  Preprocessor Lexer  |
| (Directory Discovery)|  |   (JSON Ingestion)   |  |   (#include Parser)  |
+-----------+----------+  +----------+-----------+  +-----------------+----+
            |                        |                            |
            +------------------------+----------------------------+
                                     |
                          +----------v-----------+
                          |   Dependency Graph   |
                          | (Fan-In/Out & Cycles)|
                          +----------+-----------+
                                     |
         +---------------------------+---------------------------+
         |                           |                           |
+--------v-----------+    +----------v-----------+    +----------v-----------+
|  Hotspot Scorer    |    | BuildConfig Health   |    | TU Cost Analyzer     |
| (Multi-Variate 0-100)   | (Flag Consistency)   |    | (Resource Classifier)|
+--------+-----------+    +----------+-----------+    +----------+-----------+
         |                           |                           |
         +---------------------------+---------------------------+
                                     |
                          +----------v-----------+
                          |  Build Health Score  |
                          |    (0-100 Rating)    |
                          +----------+-----------+
                                     |
                          +----------v-----------+
                          |  Report Generators   |
                          | (Terminal/JSON/HTML) |
                          +----------------------+
```

## Subsystem Design

1. **Core Foundation (`compileforge::Result`, `compileforge::JsonValue`)**:
   - Monadic `Result<T, Error>` error handling.
   - Handcrafted fast JSON tokenizer, parser, and serializer operating at >200 MB/s throughput.

2. **Project Scanner (`compileforge::ProjectScanner`)**:
   - Recursive directory scanner with `.compileforgeignore` glob matching (`*.o`, `build/*`) and deterministic relative path sorting.

3. **Compilation Database (`compileforge::CompilationDatabase`, `CompilerInvocationAnalyzer`)**:
   - GCC, Clang, and MSVC compiler flag parser handling `-I`, `/I`, `-isystem`, `/imsvc`, `-D`, `-U`, `-include`, `/FI`, `-std=`, `-O`.

4. **Preprocessor Include Parser (`compileforge::IncludeParser`)**:
   - Lexical preprocessor parser skipping `#if 0 ... #endif` blocks, detecting `#pragma once`, header guards, `#include` directives, self-includes, and duplicate inclusions.

5. **Dependency Graph & Cycle Detector (`compileforge::DependencyGraph`, `CycleDetector`)**:
   - Directed graph representation calculating fan-in, fan-out, transitive impact score, and Tarjan's Strongly Connected Components for circular include loops.

6. **Build Configuration Health & Cost Analyzer**:
   - Cross-TU flag consistency checker (detecting mixed `-O` settings or standards) and TU cost profile classifier (`Low`, `Medium`, `High`, `Critical`).

7. **Multi-Format Reporters**:
   - Auto-color terminal reporter, schema 1.0 JSON exporter, and zero-CDN dark mode HTML dashboard.
