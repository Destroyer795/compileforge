#include <compileforge/parser/compilation_database.hpp>
#include <compileforge/core/utils.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace compileforge {

std::vector<std::string> CompilationDatabase::tokenize_command_line(const std::string& cmd) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    char quote_char = 0;

    for (size_t i = 0; i < cmd.size(); ++i) {
        char c = cmd[i];
        if (in_quotes) {
            if (c == quote_char) {
                in_quotes = false;
            } else if (c == '\\' && i + 1 < cmd.size()) {
                current += cmd[++i];
            } else {
                current += c;
            }
        } else {
            if (c == '"' || c == '\'') {
                in_quotes = true;
                quote_char = c;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }
    return tokens;
}

CompileCommandEntry CompilationDatabase::parse_command_entry(
    const std::string& directory,
    const std::string& command_line,
    const std::vector<std::string>& arguments,
    const std::string& file_path,
    const std::string& output_path
) {
    CompileCommandEntry entry;
    entry.directory = utils::normalize_path(directory);
    entry.command = command_line;
    entry.file = utils::normalize_path(file_path);
    entry.output = utils::normalize_path(output_path);

    std::vector<std::string> args = arguments;
    if (args.empty() && !command_line.empty()) {
        args = tokenize_command_line(command_line);
    }
    entry.arguments = args;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];

        // Include directory flags: -I, /I, -isystem, /imsvc
        if (utils::starts_with(arg, "-I") || utils::starts_with(arg, "/I")) {
            std::string path_part = arg.substr(2);
            if (path_part.empty() && i + 1 < args.size()) {
                path_part = args[++i];
            }
            if (!path_part.empty()) {
                std::filesystem::path inc_p(path_part);
                if (inc_p.is_relative() && !entry.directory.empty()) {
                    inc_p = std::filesystem::path(entry.directory) / inc_p;
                }
                entry.include_dirs.push_back(utils::normalize_path(inc_p.string()));
            }
        } else if (arg == "-isystem" || arg == "/imsvc") {
            if (i + 1 < args.size()) {
                std::string path_part = args[++i];
                std::filesystem::path inc_p(path_part);
                if (inc_p.is_relative() && !entry.directory.empty()) {
                    inc_p = std::filesystem::path(entry.directory) / inc_p;
                }
                entry.include_dirs.push_back(utils::normalize_path(inc_p.string()));
            }
        }
        // Define flags: -D, /D
        else if (utils::starts_with(arg, "-D") || utils::starts_with(arg, "/D")) {
            std::string def_part = arg.substr(2);
            if (def_part.empty() && i + 1 < args.size()) {
                def_part = args[++i];
            }
            if (!def_part.empty()) {
                entry.defines.push_back(def_part);
            }
        }
        // Forced include flags: -include, /FI
        else if (arg == "-include" || arg == "/FI") {
            if (i + 1 < args.size()) {
                entry.forced_includes.push_back(utils::normalize_path(args[++i]));
            }
        }
    }

    return entry;
}

Result<CompilationDatabase> CompilationDatabase::load_file(const std::string& json_filepath) {
    std::ifstream ifs(json_filepath);
    if (!ifs) {
        return Error{
            ErrorCode::FileNotFound,
            "Could not open compilation database file: " + json_filepath,
            "CompilationDatabase::load_file"
        };
    }
    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto parse_res = JsonValue::parse(content);
    if (parse_res.is_error()) {
        return parse_res.error();
    }
    return load_json(parse_res.value());
}

Result<CompilationDatabase> CompilationDatabase::load_json(const JsonValue& json_root) {
    if (!json_root.is_array()) {
        return Error{
            ErrorCode::InvalidCompilationDatabase,
            "Compilation database JSON root must be an array of objects",
            "CompilationDatabase::load_json"
        };
    }

    CompilationDatabase db;
    const auto& arr = json_root.as_array();
    for (const auto& item : arr) {
        if (!item.is_object()) continue;

        std::string dir = item["directory"].as_string();
        std::string cmd = item["command"].as_string();
        std::string file = item["file"].as_string();
        std::string output = item["output"].as_string();

        std::vector<std::string> args;
        if (item["arguments"].is_array()) {
            for (const auto& arg_val : item["arguments"].as_array()) {
                args.push_back(arg_val.as_string());
            }
        }

        if (file.empty()) continue;

        CompileCommandEntry entry = parse_command_entry(dir, cmd, args, file, output);
        db.file_to_entry_index_[entry.file] = db.entries_.size();
        db.entries_.push_back(std::move(entry));
    }

    return db;
}

const CompileCommandEntry* CompilationDatabase::find_entry(const std::string& filepath) const {
    std::string norm = utils::normalize_path(filepath);
    auto it = file_to_entry_index_.find(norm);
    if (it != file_to_entry_index_.end()) {
        return &entries_[it->second];
    }
    return nullptr;
}

} // namespace compileforge
