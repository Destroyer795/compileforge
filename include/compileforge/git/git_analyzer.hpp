#ifndef COMPILEFORGE_GIT_GIT_ANALYZER_HPP
#define COMPILEFORGE_GIT_GIT_ANALYZER_HPP

#include <string>
#include <vector>
#include "../core/types.hpp"

namespace compileforge {

enum class FileChangeKind {
    Added,
    Modified,
    Deleted,
    Renamed
};

std::string file_change_kind_to_string(FileChangeKind kind);

struct ChangedFileEntry {
    std::string relative_path;
    std::string old_path; // Only for renames
    FileChangeKind change_kind{FileChangeKind::Modified};
};

class GitAnalyzer {
public:
    static GitChurnData analyze_file(const std::string& repo_path, const std::string& relative_path);
    static std::vector<ChangedFileEntry> get_changed_files(const std::string& repo_path, const std::string& revision_range = "HEAD~1..HEAD");
    static bool is_git_repository(const std::string& repo_path);
};

} // namespace compileforge

#endif // COMPILEFORGE_GIT_GIT_ANALYZER_HPP
