#include <compileforge/core/utils.hpp>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <array>

#if defined(_WIN32) || defined(_WIN64)
#define POPEN _popen
#define PCLOSE _pclose
#else
#define POPEN popen
#define PCLOSE pclose
#endif

namespace compileforge::utils {

std::string to_lower(std::string_view str) {
    std::string result(str);
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

std::string trim(std::string_view str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    return std::string(str.substr(start, end - start));
}

std::vector<std::string> split(std::string_view str, char delimiter) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string_view::npos) {
        tokens.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    tokens.emplace_back(str.substr(start));
    return tokens;
}

bool starts_with(std::string_view str, std::string_view prefix) {
    return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
}

bool ends_with(std::string_view str, std::string_view suffix) {
    return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
}

std::string normalize_path(const std::string& path) {
    if (path.empty()) return "";
    try {
        std::filesystem::path p(path);
        std::filesystem::path canonical_p = std::filesystem::weakly_canonical(p);
        std::string res = canonical_p.generic_string();
        return res;
    } catch (...) {
        std::string res = path;
        std::replace(res.begin(), res.end(), '\\', '/');
        return res;
    }
}

std::string to_relative_path(const std::string& full_path, const std::string& root_path) {
    if (full_path.empty()) return "";
    try {
        std::filesystem::path f(full_path);
        std::filesystem::path r(root_path);
        std::filesystem::path rel = std::filesystem::relative(f, r);
        std::string res = rel.generic_string();
        if (starts_with(res, "./")) return res.substr(2);
        return res;
    } catch (...) {
        return full_path;
    }
}

bool is_header_file(const std::string& path) {
    std::string ext = to_lower(std::filesystem::path(path).extension().string());
    return ext == ".h" || ext == ".hpp" || ext == ".hh" || ext == ".hxx" || ext == ".h++" || ext == ".inl";
}

bool is_source_file(const std::string& path) {
    std::string ext = to_lower(std::filesystem::path(path).extension().string());
    return ext == ".cpp" || ext == ".c" || ext == ".cc" || ext == ".cxx" || ext == ".c++";
}

FileKind classify_file_kind(const std::string& path) {
    if (is_header_file(path)) return FileKind::Header;
    if (is_source_file(path)) return FileKind::TranslationUnit;
    return FileKind::Other;
}

std::string calculate_file_hash(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) return "0000000000000000";

    // FNV-1a 64-bit hash
    uint64_t hash = 14695981039346656037ULL;
    constexpr uint64_t fnv_prime = 1099511628211ULL;

    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        std::streamsize bytes = file.gcount();
        for (std::streamsize i = 0; i < bytes; ++i) {
            hash ^= static_cast<uint8_t>(buffer[i]);
            hash *= fnv_prime;
        }
    }

    char hex_str[17];
    std::snprintf(hex_str, sizeof(hex_str), "%016llx", static_cast<unsigned long long>(hash));
    return std::string(hex_str);
}

std::string execute_command(const std::string& command, int* exit_code) {
    std::string output;
    std::array<char, 256> buffer;
    FILE* pipe = POPEN(command.c_str(), "r");
    if (!pipe) {
        if (exit_code) *exit_code = -1;
        return "";
    }
    while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
        output += buffer.data();
    }
    int code = PCLOSE(pipe);
    if (exit_code) *exit_code = code;
    return output;
}

} // namespace compileforge::utils
