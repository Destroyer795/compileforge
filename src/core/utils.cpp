#include "../../include/compileforge/core/utils.hpp"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <memory>
#include <array>

#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#define POPEN _popen
#define PCLOSE _pclose
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define POPEN popen
#define PCLOSE pclose
#define ISATTY isatty
#define FILENO fileno
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

bool glob_match(std::string_view pattern, std::string_view text) {
    size_t p = 0, t = 0;
    size_t star_p = std::string_view::npos, star_t = std::string_view::npos;

    while (t < text.size()) {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t])) {
            ++p;
            ++t;
        } else if (p < pattern.size() && pattern[p] == '*') {
            star_p = p++;
            star_t = t;
        } else if (star_p != std::string_view::npos) {
            p = star_p + 1;
            t = ++star_t;
        } else {
            return false;
        }
    }

    while (p < pattern.size() && pattern[p] == '*') {
        ++p;
    }

    return p == pattern.size();
}

std::string normalize_path(const std::string& path) {
    if (path.empty()) return "";
    std::string res;
    try {
        std::filesystem::path p(path);
        std::filesystem::path normal_p = p.lexically_normal();
        res = normal_p.generic_string();
    } catch (...) {
        res = path;
        std::replace(res.begin(), res.end(), '\\', '/');
    }

    // Windows drive letter normalization (e.g. c:/ -> C:/)
    if (res.size() >= 2 && res[1] == ':') {
        res[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(res[0])));
    }
    return res;
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

bool path_equals(const std::string& path1, const std::string& path2) {
    std::string n1 = normalize_path(path1);
    std::string n2 = normalize_path(path2);
    if ((n1.size() >= 2 && n1[1] == ':') || (n2.size() >= 2 && n2[1] == ':')) {
        return to_lower(n1) == to_lower(n2);
    }
#if defined(_WIN32) || defined(_WIN64)
    return to_lower(n1) == to_lower(n2);
#else
    return n1 == n2;
#endif
}

bool is_header_file(const std::string& path) {
    std::string ext = to_lower(std::filesystem::path(path).extension().string());
    return ext == ".h" || ext == ".hpp" || ext == ".hh" || ext == ".hxx" || ext == ".h++" || ext == ".inl" || ext == ".ipp" || ext == ".tpp";
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
    std::array<char, 4096> buffer;
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

bool is_atty() {
    return ISATTY(FILENO(stdout)) != 0;
}

} // namespace compileforge::utils
