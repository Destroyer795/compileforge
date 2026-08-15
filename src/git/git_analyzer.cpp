#include "../../include/compileforge/git/git_analyzer.hpp"
#include "../../include/compileforge/core/utils.hpp"
#include <sstream>
#include <filesystem>

namespace compileforge {

std::string file_change_kind_to_string(FileChangeKind kind) {
    switch (kind) {
        case FileChangeKind::Added: return "Added";
        case FileChangeKind::Modified: return "Modified";
        case FileChangeKind::Deleted: return "Deleted";
        case FileChangeKind::Renamed: return "Renamed";
        default: return "Modified";
    }
}

bool GitAnalyzer::is_git_repository(const std::string& repo_path) {
    std::filesystem::path git_dir = std::filesystem::path(repo_path) / ".git";
    return std::filesystem::exists(git_dir);
}

GitChurnData GitAnalyzer::analyze_file(const std::string& repo_path, const std::string& relative_path) {
    GitChurnData metrics;
    if (!is_git_repository(repo_path)) return metrics;

    std::string cmd = "git -C \"" + repo_path + "\" log --follow --oneline -- \"" + relative_path + "\"";
    int exit_code = 0;
    std::string output = utils::execute_command(cmd, &exit_code);

    if (exit_code != 0 || output.empty()) return metrics;

    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        if (!utils::trim(line).empty()) {
            metrics.commit_count++;
        }
    }

    return metrics;
}

std::vector<ChangedFileEntry> GitAnalyzer::get_changed_files(const std::string& repo_path, const std::string& rev_range) {
    std::vector<ChangedFileEntry> changed_files;
    if (!is_git_repository(repo_path)) return changed_files;

    std::string cmd;
    if (rev_range.empty() || rev_range == "working") {
        cmd = "git -C \"" + repo_path + "\" diff --name-status -M";
    } else if (rev_range == "staged") {
        cmd = "git -C \"" + repo_path + "\" diff --name-status -M --cached";
    } else {
        cmd = "git -C \"" + repo_path + "\" diff --name-status -M " + rev_range;
    }

    int exit_code = 0;
    std::string output = utils::execute_command(cmd, &exit_code);
    if (exit_code != 0 || output.empty()) return changed_files;

    std::istringstream iss(output);
    std::string line;
    while (std::getline(iss, line)) {
        std::string trimmed = utils::trim(line);
        if (trimmed.empty()) continue;

        auto tokens = utils::split(trimmed, '\t');
        if (tokens.empty()) continue;

        std::string status = utils::trim(tokens[0]);
        ChangedFileEntry entry;

        if (status[0] == 'A') {
            entry.change_kind = FileChangeKind::Added;
            if (tokens.size() >= 2) entry.relative_path = utils::trim(tokens[1]);
        } else if (status[0] == 'D') {
            entry.change_kind = FileChangeKind::Deleted;
            if (tokens.size() >= 2) entry.relative_path = utils::trim(tokens[1]);
        } else if (status[0] == 'R') {
            entry.change_kind = FileChangeKind::Renamed;
            if (tokens.size() >= 3) {
                entry.old_path = utils::trim(tokens[1]);
                entry.relative_path = utils::trim(tokens[2]);
            } else if (tokens.size() >= 2) {
                entry.relative_path = utils::trim(tokens[1]);
            }
        } else { // Modified or fallback
            entry.change_kind = FileChangeKind::Modified;
            if (tokens.size() >= 2) entry.relative_path = utils::trim(tokens[1]);
        }

        if (!entry.relative_path.empty()) {
            changed_files.push_back(entry);
        }
    }

    return changed_files;
}

} // namespace compileforge
