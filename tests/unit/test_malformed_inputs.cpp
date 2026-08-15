#include "../test_framework.hpp"
#include "../../include/compileforge/core/json.hpp"
#include "../../include/compileforge/parser/compilation_database.hpp"
#include "../../include/compileforge/parser/include_parser.hpp"
#include "../../include/compileforge/core/utils.hpp"

using namespace compileforge;

TEST_CASE(test_malformed_json_resilience) {
    auto res1 = JsonValue::parse("[{ \"directory\": \"/build\", \"command\": \"g++\", }]"); // trailing comma
    // Should parse cleanly with comment/comma tolerance or fail with clean ParseError
    if (res1.is_error()) {
        ASSERT_EQ(res1.error().code, ErrorCode::ParseError);
    }

    auto res2 = JsonValue::parse("INVALID_NON_JSON_CONTENT");
    ASSERT_TRUE(res2.is_error());
    ASSERT_EQ(res2.error().code, ErrorCode::ParseError);
}

TEST_CASE(test_missing_compilation_database_diagnostic) {
    auto db_res = CompilationDatabase::load_file("non_existent_compile_commands.json");
    ASSERT_TRUE(db_res.is_error());
    ASSERT_EQ(db_res.error().code, ErrorCode::FileNotFound);
    ASSERT_TRUE(db_res.error().message.find("CMAKE_EXPORT_COMPILE_COMMANDS") != std::string::npos);
}

TEST_CASE(test_if_zero_code_block_skipping) {
    std::string code = R"(
#pragma once
#include "active.hpp"

#if 0
#include "disabled.hpp"
#endif

#include "active.hpp" // duplicate include
)";

    ParseFileResult res = IncludeParser::parse_content(code, "test_file.hpp");
    ASSERT_EQ(res.includes.size(), 2);
    ASSERT_EQ(res.includes[0].raw_path, "active.hpp");
    ASSERT_EQ(res.includes[1].raw_path, "active.hpp");
    ASSERT_TRUE(res.includes[1].is_duplicate);
}

TEST_CASE(test_self_including_header_detection) {
    std::string code = R"(
#pragma once
#include "self_header.hpp"
)";

    ParseFileResult res = IncludeParser::parse_content(code, "include/self_header.hpp");
    ASSERT_EQ(res.includes.size(), 1);
    ASSERT_TRUE(res.includes[0].is_self_include);
}

TEST_CASE(test_path_normalization_and_spaces) {
    std::string p1 = "C:/Project/src/../include/common.hpp";
    std::string p2 = "C:/Project/include/common.hpp";

    ASSERT_TRUE(utils::path_equals(p1, p2));
}
