cmake_minimum_required(VERSION 3.20)
project(CompileForge VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for self-analysis
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(COMPILEFORGE_BUILD_TESTS "Build CompileForge unit and integration tests" ON)
option(COMPILEFORGE_BUILD_BENCHMARKS "Build CompileForge benchmarks" ON)
option(COMPILEFORGE_ENABLE_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# Compiler flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
    if(COMPILEFORGE_ENABLE_WARNINGS_AS_ERRORS)
        add_compile_options(/WX)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wshadow -Wnon-virtual-dtor -Wconversion)
    if(COMPILEFORGE_ENABLE_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()
endif()

# Core Library Sources
set(COMPILEFORGE_HEADERS
    include/compileforge/core/result.hpp
    include/compileforge/core/types.hpp
    include/compileforge/core/utils.hpp
    include/compileforge/core/json.hpp
    include/compileforge/project/scanner.hpp
    include/compileforge/parser/compilation_database.hpp
    include/compileforge/parser/include_parser.hpp
    include/compileforge/graph/dependency_graph.hpp
    include/compileforge/graph/cycle_detector.hpp
    include/compileforge/metrics/source_metrics.hpp
    include/compileforge/build/build_trace.hpp
    include/compileforge/git/git_analyzer.hpp
    include/compileforge/analysis/hotspot_scorer.hpp
    include/compileforge/analysis/regression_detector.hpp
    include/compileforge/recommendations/recommendation_engine.hpp
    include/compileforge/config/config.hpp
    include/compileforge/cache/analysis_cache.hpp
    include/compileforge/reporting/report.hpp
    include/compileforge/reporting/terminal_reporter.hpp
    include/compileforge/reporting/json_reporter.hpp
    include/compileforge/reporting/html_reporter.hpp
)

set(COMPILEFORGE_SOURCES
    src/core/utils.cpp
    src/core/json.cpp
    src/project/scanner.cpp
    src/parser/compilation_database.cpp
    src/parser/include_parser.cpp
    src/graph/dependency_graph.cpp
    src/graph/cycle_detector.cpp
    src/metrics/source_metrics.cpp
    src/build/build_trace.cpp
    src/git/git_analyzer.cpp
    src/analysis/hotspot_scorer.cpp
    src/analysis/regression_detector.cpp
    src/recommendations/recommendation_engine.cpp
    src/config/config.cpp
    src/cache/analysis_cache.cpp
    src/reporting/terminal_reporter.cpp
    src/reporting/json_reporter.cpp
    src/reporting/html_reporter.cpp
)

add_library(compileforge_lib STATIC ${COMPILEFORGE_HEADERS} ${COMPILEFORGE_SOURCES})

target_include_directories(compileforge_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# CLI Binary
add_executable(compileforge src/main.cpp)
target_link_libraries(compileforge PRIVATE compileforge_lib)
target_include_directories(compileforge PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Testing
if(COMPILEFORGE_BUILD_TESTS)
    enable_testing()
    add_executable(compileforge_tests
        tests/test_main.cpp
        tests/unit/test_json.cpp
        tests/unit/test_scanner.cpp
        tests/unit/test_compilation_database.cpp
        tests/unit/test_include_parser.cpp
        tests/unit/test_dependency_graph.cpp
        tests/unit/test_cycle_detector.cpp
        tests/unit/test_metrics.cpp
        tests/unit/test_hotspot.cpp
        tests/unit/test_recommendations.cpp
        tests/unit/test_regression.cpp
        tests/integration/test_end_to_end.cpp
    )
    target_link_libraries(compileforge_tests PRIVATE compileforge_lib)
    target_include_directories(compileforge_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    add_test(NAME compileforge_all_tests COMMAND compileforge_tests)
endif()

# Benchmarks
if(COMPILEFORGE_BUILD_BENCHMARKS)
    add_executable(compileforge_benchmarks benchmarks/benchmark_main.cpp)
    target_link_libraries(compileforge_benchmarks PRIVATE compileforge_lib)
    target_include_directories(compileforge_benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
