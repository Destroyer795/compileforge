#ifndef COMPILEFORGE_PARSER_COMPILATION_DATABASE_HPP
#define COMPILEFORGE_PARSER_COMPILATION_DATABASE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <compileforge/core/result.hpp>
#include <compileforge/core/types.hpp>
#include <compileforge/core/json.hpp>

namespace compileforge {

class CompilationDatabase {
public:
    static Result<CompilationDatabase> load_file(const std::string& json_filepath);
    static Result<CompilationDatabase> load_json(const JsonValue& json_root);

    [[nodiscard]] const std::vector<CompileCommandEntry>& entries() const { return entries_; }
    [[nodiscard]] const CompileCommandEntry* find_entry(const std::string& filepath) const;

    static CompileCommandEntry parse_command_entry(
        const std::string& directory,
        const std::string& command_line,
        const std::vector<std::string>& arguments,
        const std::string& file_path,
        const std::string& output_path
    );

    static std::vector<std::string> tokenize_command_line(const std::string& cmd);

private:
    std::vector<CompileCommandEntry> entries_;
    std::unordered_map<std::string, size_t> file_to_entry_index_;
};

} // namespace compileforge

#endif // COMPILEFORGE_PARSER_COMPILATION_DATABASE_HPP
