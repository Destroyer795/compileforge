#include <tests/test_framework.hpp>
#include <compileforge/analysis/build_config_health.hpp>

using namespace compileforge;

TEST_CASE(test_build_config_health_inconsistent_flags) {
    CompilationDatabase db;
    // Entry 1 uses -O2
    CompileCommandEntry e1;
    e1.file = "src/a.cpp";
    e1.arguments = {"g++", "-O2", "-std=c++20", "-c", "src/a.cpp"};

    // Entry 2 uses -O0
    CompileCommandEntry e2;
    e2.file = "src/b.cpp";
    e2.arguments = {"g++", "-O0", "-std=c++20", "-c", "src/b.cpp"};

    db.add_entry(e1);
    db.add_entry(e2);

    auto findings = BuildConfigHealthAnalyzer::analyze(db);
    ASSERT_TRUE(!findings.empty());
    ASSERT_EQ(findings[0].category, "Optimization Inconsistency");
}
