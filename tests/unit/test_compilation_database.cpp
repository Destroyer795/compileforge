#include "../test_framework.hpp"
#include "../../include/compileforge/parser/compilation_database.hpp"

using namespace compileforge;

TEST_CASE(test_compilation_database_parsing) {
    std::string json_str = R"([
        {
            "directory": "/home/user/project",
            "command": "g++ -Iinclude -I/usr/local/include -DFOO=1 -c src/main.cpp -o main.o",
            "file": "src/main.cpp",
            "output": "main.o"
        },
        {
            "directory": "/home/user/project",
            "arguments": ["clang++", "-I", "include/core", "-DBAR", "-c", "src/render.cpp"],
            "file": "src/render.cpp"
        }
    ])";

    auto parse_res = JsonValue::parse(json_str);
    ASSERT_TRUE(parse_res.has_value());

    auto db_res = CompilationDatabase::load_json(parse_res.value());
    ASSERT_TRUE(db_res.has_value());

    const auto& db = db_res.value();
    ASSERT_EQ(db.entries().size(), 2);

    const auto* entry1 = db.find_entry("/home/user/project/src/main.cpp");
    if (!entry1) entry1 = db.find_entry("src/main.cpp");
    ASSERT_TRUE(entry1 != nullptr);
    ASSERT_EQ(entry1->include_dirs.size(), 2);
    ASSERT_EQ(entry1->defines.size(), 1);
    ASSERT_EQ(entry1->defines[0], "FOO=1");

    const auto* entry2 = db.find_entry("/home/user/project/src/render.cpp");
    if (!entry2) entry2 = db.find_entry("src/render.cpp");
    ASSERT_TRUE(entry2 != nullptr);
    ASSERT_EQ(entry2->include_dirs.size(), 1);
    ASSERT_EQ(entry2->defines[0], "BAR");
}
