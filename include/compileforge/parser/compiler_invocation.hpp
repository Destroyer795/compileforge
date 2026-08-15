#ifndef COMPILEFORGE_PARSER_COMPILER_INVOCATION_HPP
#define COMPILEFORGE_PARSER_COMPILER_INVOCATION_HPP

#include <string>
#include <vector>
#include "../core/types.hpp"

namespace compileforge {

enum class CompilerFamily {
    GCC,
    Clang,
    MSVC,
    Unknown
};

std::string compiler_family_to_string(CompilerFamily family);

struct CompilerInvocationInfo {
    CompilerFamily family{CompilerFamily::Unknown};
    std::string compiler_executable;
    std::string language_standard;
    std::string optimization_level{"O0"};
    bool has_debug_symbols{false};
    bool has_lto{false};
    bool has_sanitizers{false};
    std::vector<std::string> warning_flags;
    std::vector<std::string> include_dirs;
    std::vector<std::string> system_include_dirs;
    std::vector<std::string> defines;
    std::vector<std::string> forced_includes;
    std::string target_architecture;
};

class CompilerInvocationAnalyzer {
public:
    static CompilerInvocationInfo parse_arguments(const std::vector<std::string>& arguments);
};

} // namespace compileforge

#endif // COMPILEFORGE_PARSER_COMPILER_INVOCATION_HPP
