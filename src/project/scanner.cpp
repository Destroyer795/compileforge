#include "../../include/compileforge/project/scanner.hpp"
#include "../../include/compileforge/core/utils.hpp"
#include <fstream>
#include <algorithm>

namespace compileforge {

ProjectScanner::ProjectScanner(ScanOptions options) : options_(std::move(options)) {
    options_.root_path = utils::normalize_path(options_.root_path);
}

bool ProjectScanner::should_ignore(
    const std::filesystem::path& rel_path,
    const std::vector<std::string>& custom_ignores,
    bool ignore_hidden
) {
    std::string path_str = rel_path.generic_string();
    if (path_str.empty() || path_str == ".") return false;

    // Check hidden files/directories
    if (ignore_hidden) {
        for (const auto& part : rel_path) {
            std::string part_str = part.string();
            if (part_str.size() > 1 && part_str[0] == '.' && part_str != ".." && part_str != ".") {
                return true;
            }
        }
    }

    // Standard ignores
    static const std::vector<std::string> default_ignores = {
        "build", "cmake-build-debug", "cmake-build-release", ".git", ".vs",
        ".cache", "node_modules", "out", "target", "bin", "obj", ".compileforge.cache"
    };

    for (const auto& ign : default_ignores) {
        for (const auto& part : rel_path) {
            if (utils::to_lower(part.string()) == ign) {
                return true;
            }
        }
    }

    for (const auto& custom_ign : custom_ignores) {
        if (custom_ign.empty()) continue;
        if (utils::glob_match(custom_ign, path_str) || utils::starts_with(path_str, custom_ign)) {
            return true;
        }
    }

    return false;
}

Result<std::vector<FileNode>> ProjectScanner::scan() {
    std::filesystem::path root(options_.root_path);
    if (!std::filesystem::exists(root)) {
        return Error(
            ErrorCode::FileNotFound,
            "Target directory does not exist: " + options_.root_path,
            "ProjectScanner::scan"
        );
    }
    if (!std::filesystem::is_directory(root)) {
        return Error(
            ErrorCode::InvalidPath,
            "Target path is not a directory: " + options_.root_path,
            "ProjectScanner::scan"
        );
    }

    // Check for .compileforgeignore
    std::filesystem::path ignore_file = root / ".compileforgeignore";
    if (std::filesystem::exists(ignore_file)) {
        std::ifstream ifs(ignore_file);
        std::string line;
        while (std::getline(ifs, line)) {
            line = utils::trim(line);
            if (!line.empty() && line[0] != '#') {
                options_.ignore_patterns.push_back(line);
            }
        }
    }

    std::vector<FileNode> nodes;
    std::error_code ec;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(root, std::filesystem::directory_options::skip_permission_denied, ec)) {
        if (ec) continue;

        std::filesystem::path rel_path = std::filesystem::relative(entry.path(), root, ec);
        if (ec) continue;

        if (should_ignore(rel_path, options_.ignore_patterns, options_.ignore_hidden)) {
            continue;
        }

        if (!entry.is_regular_file()) continue;

        std::string full_path = utils::normalize_path(entry.path().string());
        FileKind kind = utils::classify_file_kind(full_path);

        if (kind == FileKind::Other) continue; // Skip non C/C++ files

        FileNode node;
        node.canonical_path = full_path;
        node.relative_path = utils::to_relative_path(full_path, options_.root_path);
        node.kind = kind;
        node.file_size_bytes = static_cast<uint64_t>(entry.file_size(ec));
        if (options_.calculate_hashes) {
            node.content_hash = utils::calculate_file_hash(full_path);
        }
        nodes.push_back(std::move(node));
    }

    // Deterministic sorting by relative path
    std::sort(nodes.begin(), nodes.end(), [](const FileNode& a, const FileNode& b) {
        return a.relative_path < b.relative_path;
    });

    return nodes;
}

} // namespace compileforge
