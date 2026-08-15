#include "../test_framework.hpp"
#include "../../include/compileforge/project/scanner.hpp"
#include <filesystem>
#include <fstream>

using namespace compileforge;

TEST_CASE(test_project_scanner_ignore_rules) {
    std::filesystem::path p1("build/foo.cpp");
    std::filesystem::path p2("src/main.cpp");
    std::filesystem::path p3(".git/config");

    std::vector<std::string> custom;
    ASSERT_TRUE(ProjectScanner::should_ignore(p1, custom, true));
    ASSERT_FALSE(ProjectScanner::should_ignore(p2, custom, true));
    ASSERT_TRUE(ProjectScanner::should_ignore(p3, custom, true));
}

TEST_CASE(test_project_scanner_directory_scan) {
    std::filesystem::path tmp_dir = std::filesystem::temp_directory_path() / "compileforge_test_scanner";
    std::filesystem::create_directories(tmp_dir / "src");
    std::filesystem::create_directories(tmp_dir / "include");
    std::filesystem::create_directories(tmp_dir / "build");

    {
        std::ofstream ofs(tmp_dir / "src" / "main.cpp");
        ofs << "#include \"include/app.hpp\"\nint main() { return 0; }\n";
    }
    {
        std::ofstream ofs(tmp_dir / "include" / "app.hpp");
        ofs << "#pragma once\nvoid run();\n";
    }
    {
        std::ofstream ofs(tmp_dir / "build" / "ignored.cpp");
        ofs << "// build artifact\n";
    }

    ScanOptions opts;
    opts.root_path = tmp_dir.string();
    ProjectScanner scanner(opts);

    auto scan_res = scanner.scan();
    ASSERT_TRUE(scan_res.has_value());

    const auto& nodes = scan_res.value();
    ASSERT_EQ(nodes.size(), 2);

    bool found_main = false;
    bool found_app = false;
    for (const auto& n : nodes) {
        if (n.relative_path == "src/main.cpp") {
            found_main = true;
            ASSERT_EQ(n.kind, FileKind::TranslationUnit);
        }
        if (n.relative_path == "include/app.hpp") {
            found_app = true;
            ASSERT_EQ(n.kind, FileKind::Header);
        }
    }
    ASSERT_TRUE(found_main);
    ASSERT_TRUE(found_app);

    std::error_code ec;
    std::filesystem::remove_all(tmp_dir, ec);
}
