#include <compileforge/git/git_analyzer.hpp>
#include <compileforge/core/utils.hpp>
#include <sstream>
#include <iostream>
#include <unordered_set>

namespace compileforge {

bool GitAnalyzer::is_git_repository(const std::string& repository_root) {
    int code = 0;
    std::string cmd = "git -C \"" + repository_root + "\" rev-parse --is-inside-work-tree";
    std::string out = utils::execute_command(cmd, &code);
    return code == 0 && utils::trim(out) == "true";
}

Result<std::unordered_map<std::string, GitChurnData>> GitAnalyzer::analyze_repository(const std::string& repository_root) {
    if (!is_git_repository(repository_root)) {
        return Error{
            ErrorCode::GitError,
            "Target path is not a valid Git repository: " + repository_root,
            "GitAnalyzer::analyze_repository"
        };
    }

    std::string cmd = "git -C \"" + repository_root + "\" log --numstat --format=\"COMMIT|%h|%an|%at\" --no-merges -n 500";
    int code = 0;
    std::string output = utils::execute_command(cmd, &code);
    if (code != 0 || output.empty()) {
        return Error{
            ErrorCode::GitError,
            "Failed to execute git log on repository: " + repository_root,
            "GitAnalyzer::analyze_repository"
        };
    }

    std::unordered_map<std::string, GitChurnData> churn_map;
    std::unordered_map<std::string, std::unordered_set<std::string>> file_authors;

    std::istringstream iss(output);
    std::string line;
    std::string current_author;

    while (std::getline(iss, line)) {
        line = utils::trim(line);
        if (line.empty()) continue;

        if (utils::starts_with(line, "COMMIT|")) {
            auto parts = utils::split(line, '|');
            if (parts.size() >= 3) {
                current_author = parts[2];
            }
        } else {
            auto parts = utils::split(line, '\t');
            if (parts.size() >= 3) {
                std::string added_str = parts[0];
                std::string deleted_str = parts[1];
                std::string rel_file_path = utils::normalize_path(parts[2]);

                size_t added = (added_str != "-") ? std::stoull(added_str) : 0;
                size_t deleted = (deleted_str != "-") ? std::stoull(deleted_str) : 0;

                auto& data = churn_map[rel_file_path];
                data.git_tracked = true;
                data.commit_count++;
                data.added_lines += added;
                data.deleted_lines += deleted;

                if (!current_author.empty()) {
                    file_authors[rel_file_path].insert(current_author);
                }
            }
        }
    }

    for (auto& [path, data] : churn_map) {
        data.author_count = file_authors[path].size();
    }

    return churn_map;
}

void GitAnalyzer::enrich_graph(DependencyGraph& graph, const std::string& repository_root) {
    auto churn_res = analyze_repository(repository_root);
    if (churn_res.is_error()) return;

    const auto& churn_map = churn_res.value();
    for (const auto& path : graph.all_nodes()) {
        auto* node = graph.get_node_mut(path);
        if (!node) continue;

        auto it = churn_map.find(node->relative_path);
        if (it != churn_map.end()) {
            node->git_data = it->second;
        }
    }
}

} // namespace compileforge
