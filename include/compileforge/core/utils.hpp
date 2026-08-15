#ifndef COMPILEFORGE_CORE_UTILS_HPP
#define COMPILEFORGE_CORE_UTILS_HPP

#include <string>
#include <string_view>
#include <vector>
#include <filesystem>
#include <compileforge/core/types.hpp>

namespace compileforge::utils {

std::string to_lower(std::string_view str);
std::string trim(std::string_view str);
std::vector<std::string> split(std::string_view str, char delimiter);
bool starts_with(std::string_view str, std::string_view prefix);
bool ends_with(std::string_view str, std::string_view suffix);
bool glob_match(std::string_view pattern, std::string_view text);

std::string normalize_path(const std::string& path);
std::string to_relative_path(const std::string& full_path, const std::string& root_path);
bool path_equals(const std::string& path1, const std::string& path2);

FileKind classify_file_kind(const std::string& path);
bool is_header_file(const std::string& path);
bool is_source_file(const std::string& path);

std::string calculate_file_hash(const std::string& filepath);
std::string execute_command(const std::string& command, int* exit_code = nullptr);
bool is_atty();

} // namespace compileforge::utils

#endif // COMPILEFORGE_CORE_UTILS_HPP
