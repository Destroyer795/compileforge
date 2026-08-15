#ifndef COMPILEFORGE_PROJECT_SCANNER_HPP
#define COMPILEFORGE_PROJECT_SCANNER_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include <filesystem>
#include "../core/result.hpp"
#include "../core/types.hpp"

namespace compileforge {

struct ScanOptions {
    std::string root_path;
    std::vector<std::string> ignore_patterns;
    std::vector<std::string> source_extensions{".cpp", ".c", ".cc", ".cxx", ".c++"};
    std::vector<std::string> header_extensions{".hpp", ".h", ".hh", ".hxx", ".h++", ".inl"};
    bool ignore_hidden{true};
    bool calculate_hashes{true};
};

class ProjectScanner {
public:
    explicit ProjectScanner(ScanOptions options);

    [[nodiscard]] Result<std::vector<FileNode>> scan();

    [[nodiscard]] static bool should_ignore(
        const std::filesystem::path& rel_path,
        const std::vector<std::string>& custom_ignores,
        bool ignore_hidden
    );

private:
    ScanOptions options_;
};

} // namespace compileforge

#endif // COMPILEFORGE_PROJECT_SCANNER_HPP
