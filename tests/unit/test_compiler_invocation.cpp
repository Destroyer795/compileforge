#include "../test_framework.hpp"
#include "../../include/compileforge/parser/compiler_invocation.hpp"

using namespace compileforge;

TEST_CASE(test_compiler_invocation_parsing) {
    std::vector<std::string> args = {
        "g++", "-std=c++20", "-O3", "-Wall", "-Wextra", "-Iinclude", "-isystem", "/usr/include", "-DNDEBUG", "-flto"
    };

    auto info = CompilerInvocationAnalyzer::parse_arguments(args);
    ASSERT_EQ(compiler_family_to_string(info.family), "GCC");
    ASSERT_EQ(info.language_standard, "c++20");
    ASSERT_EQ(info.optimization_level, "3");
    ASSERT_TRUE(info.has_lto);
    ASSERT_EQ(info.include_dirs.size(), 1);
    ASSERT_EQ(info.system_include_dirs.size(), 1);
    ASSERT_EQ(info.defines.size(), 1);
    ASSERT_EQ(info.defines[0], "NDEBUG");
}
