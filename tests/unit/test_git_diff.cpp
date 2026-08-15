#include "../test_framework.hpp"
#include "../../include/compileforge/git/git_analyzer.hpp"

using namespace compileforge;

TEST_CASE(test_git_diff_non_repo_resilience) {
    auto files = GitAnalyzer::get_changed_files("non_existent_dir_path");
    ASSERT_TRUE(files.empty());
}
