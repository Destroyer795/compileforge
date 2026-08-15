#include <compileforge/parser/include_parser.hpp>
#include <compileforge/core/utils.hpp>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace compileforge {

Result<ParseFileResult> IncludeParser::parse_file(const std::string& filepath) {
    std::ifstream ifs(filepath, std::ios::binary);
    if (!ifs) {
        return Error(
            ErrorCode::FileNotFound,
            "Could not open source file: " + filepath,
            "IncludeParser::parse_file"
        );
    }
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    return parse_content(content, filepath);
}

ParseFileResult IncludeParser::parse_content(const std::string& content, const std::string& filepath) {
    ParseFileResult result;
    std::istringstream iss(content);
    std::string line;

    bool in_block_comment = false;
    int if_zero_depth = 0;
    size_t line_num = 0;
    std::string last_ifndef_guard;
    std::unordered_set<std::string> seen_includes;

    std::string current_file_basename = utils::to_lower(std::filesystem::path(filepath).filename().string());

    while (std::getline(iss, line)) {
        ++line_num;
        result.metrics.total_lines++;

        std::string trimmed = utils::trim(line);
        if (trimmed.empty()) {
            result.metrics.blank_lines++;
            continue;
        }

        // Handle multi-line block comments /* ... */
        std::string code_line;
        size_t pos = 0;
        while (pos < line.size()) {
            if (in_block_comment) {
                size_t end_comment = line.find("*/", pos);
                if (end_comment != std::string::npos) {
                    in_block_comment = false;
                    pos = end_comment + 2;
                } else {
                    result.metrics.comment_lines++;
                    break;
                }
            } else {
                size_t start_comment = line.find("/*", pos);
                size_t line_comment = line.find("//", pos);

                if (line_comment != std::string::npos && (start_comment == std::string::npos || line_comment < start_comment)) {
                    code_line += line.substr(pos, line_comment - pos);
                    result.metrics.comment_lines++;
                    break; // Skip rest of line
                } else if (start_comment != std::string::npos) {
                    code_line += line.substr(pos, start_comment - pos);
                    in_block_comment = true;
                    pos = start_comment + 2;
                } else {
                    code_line += line.substr(pos);
                    break;
                }
            }
        }

        std::string clean_line = utils::trim(code_line);
        if (clean_line.empty()) {
            if (!in_block_comment) result.metrics.blank_lines++;
            continue;
        }

        // Handle #if 0 ... #endif skipping
        if (utils::starts_with(clean_line, "#if 0") || utils::starts_with(clean_line, "#if  0")) {
            if_zero_depth++;
            continue;
        }
        if (if_zero_depth > 0) {
            if (utils::starts_with(clean_line, "#if")) {
                if_zero_depth++;
            } else if (utils::starts_with(clean_line, "#endif")) {
                if_zero_depth--;
            }
            continue; // Skip lines inside #if 0 block
        }

        result.metrics.sloc++;

        // Detect cyclomatic complexity heuristics (control statements)
        if (clean_line.find("if(") != std::string::npos || clean_line.find("if ") != std::string::npos ||
            clean_line.find("else if") != std::string::npos || clean_line.find("while(") != std::string::npos ||
            clean_line.find("while ") != std::string::npos || clean_line.find("for(") != std::string::npos ||
            clean_line.find("for ") != std::string::npos || clean_line.find("switch(") != std::string::npos ||
            clean_line.find("switch ") != std::string::npos || clean_line.find("case ") != std::string::npos ||
            clean_line.find("catch(") != std::string::npos || clean_line.find("catch ") != std::string::npos) {
            result.metrics.cyclomatic_complexity++;
        }

        // Count macros and templates
        if (utils::starts_with(clean_line, "#define ")) {
            result.metrics.macro_count++;
        }
        if (clean_line.find("template<") != std::string::npos || clean_line.find("template <") != std::string::npos) {
            result.metrics.template_count++;
        }

        // Detect header guards
        if (clean_line == "#pragma once" || clean_line.find("#pragma once") == 0) {
            result.metrics.has_pragma_once = true;
        }
        if (utils::starts_with(clean_line, "#ifndef ")) {
            last_ifndef_guard = utils::trim(clean_line.substr(8));
        }
        if (!last_ifndef_guard.empty() && utils::starts_with(clean_line, "#define ")) {
            std::string def_name = utils::trim(clean_line.substr(8));
            if (def_name.find(' ') != std::string::npos) {
                def_name = def_name.substr(0, def_name.find(' '));
            }
            if (def_name == last_ifndef_guard) {
                result.metrics.has_header_guard = true;
                result.metrics.header_guard_name = def_name;
            }
        }

        // Detect #include directives
        if (utils::starts_with(clean_line, "#include")) {
            std::string inc_part = utils::trim(clean_line.substr(8));
            if (inc_part.size() >= 3) {
                char first = inc_part.front();
                char last = inc_part.back();
                if ((first == '"' && last == '"') || (first == '<' && last == '>')) {
                    IncludeDirective dir;
                    dir.raw_path = inc_part.substr(1, inc_part.size() - 2);
                    dir.kind = (first == '"') ? IncludeKind::User : IncludeKind::System;
                    dir.line_number = line_num;

                    std::string inc_basename = utils::to_lower(std::filesystem::path(dir.raw_path).filename().string());
                    if (!current_file_basename.empty() && current_file_basename == inc_basename) {
                        dir.is_self_include = true;
                    }
                    if (seen_includes.find(dir.raw_path) != seen_includes.end()) {
                        dir.is_duplicate = true;
                    } else {
                        seen_includes.insert(dir.raw_path);
                    }

                    result.includes.push_back(std::move(dir));
                }
            }
        }
    }

    return result;
}

std::string IncludeParser::resolve_include_path(
    const IncludeDirective& directive,
    const std::string& current_file_path,
    const std::vector<std::string>& include_search_dirs,
    const std::string& project_root
) {
    std::filesystem::path cur_p(current_file_path);
    std::filesystem::path cur_dir = cur_p.parent_path();
    std::filesystem::path raw_inc(directive.raw_path);

    // 1. Try relative to current file's directory (for user includes "#include ...")
    if (directive.kind == IncludeKind::User && !current_file_path.empty()) {
        std::filesystem::path cand = cur_dir / raw_inc;
        if (std::filesystem::exists(cand)) {
            return utils::normalize_path(cand.string());
        }
    }

    // 2. Try against include search directories from compilation database
    for (const auto& search_dir : include_search_dirs) {
        std::filesystem::path cand = std::filesystem::path(search_dir) / raw_inc;
        if (std::filesystem::exists(cand)) {
            return utils::normalize_path(cand.string());
        }
    }

    // 3. Try relative to project root
    if (!project_root.empty()) {
        std::filesystem::path cand = std::filesystem::path(project_root) / raw_inc;
        if (std::filesystem::exists(cand)) {
            return utils::normalize_path(cand.string());
        }
    }

    return "";
}

} // namespace compileforge
