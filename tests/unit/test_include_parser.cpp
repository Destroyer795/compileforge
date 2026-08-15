#include "../test_framework.hpp"
#include "../../include/compileforge/parser/include_parser.hpp"

using namespace compileforge;

TEST_CASE(test_include_parser_directives_and_guards) {
    std::string code = R"(
#ifndef MY_HEADER_HPP
#define MY_HEADER_HPP

#pragma once

#include "core/types.hpp"
#include <vector>
#include <iostream>

// This is a comment
/* Block comment
   with multiple lines */

#define MAX_BUFFER 1024

template <typename T>
void process(T val) {
    if (val > 0) {
        // do something
    } else if (val < 0) {
        // do other thing
    }
}

#endif // MY_HEADER_HPP
)";

    ParseFileResult res = IncludeParser::parse_content(code, "my_header.hpp");
    
    ASSERT_TRUE(res.metrics.has_pragma_once);
    ASSERT_TRUE(res.metrics.has_header_guard);
    ASSERT_EQ(res.metrics.header_guard_name, "MY_HEADER_HPP");

    ASSERT_EQ(res.includes.size(), 3);
    ASSERT_EQ(res.includes[0].raw_path, "core/types.hpp");
    ASSERT_EQ(res.includes[0].kind, IncludeKind::User);
    ASSERT_EQ(res.includes[1].raw_path, "vector");
    ASSERT_EQ(res.includes[1].kind, IncludeKind::System);

    ASSERT_EQ(res.metrics.macro_count, 2); // MY_HEADER_HPP and MAX_BUFFER
    ASSERT_TRUE(res.metrics.cyclomatic_complexity >= 2);
}
