#include <compileforge/parser/compiler_invocation.hpp>
#include <compileforge/core/utils.hpp>
#include <algorithm>

namespace compileforge {

std::string compiler_family_to_string(CompilerFamily family) {
    switch (family) {
        case CompilerFamily::GCC: return "GCC";
        case CompilerFamily::Clang: return "Clang";
        case CompilerFamily::MSVC: return "MSVC";
        default: return "Unknown";
    }
}

CompilerInvocationInfo CompilerInvocationAnalyzer::parse_arguments(const std::vector<std::string>& args) {
    CompilerInvocationInfo info;
    if (args.empty()) return info;

    // Detect compiler executable & family from first arg
    info.compiler_executable = args.front();
    std::string exe_lower = utils::to_lower(info.compiler_executable);
    if (exe_lower.find("g++") != std::string::npos || exe_lower.find("gcc") != std::string::npos) {
        info.family = CompilerFamily::GCC;
    } else if (exe_lower.find("clang") != std::string::npos) {
        info.family = CompilerFamily::Clang;
    } else if (exe_lower.find("cl.exe") != std::string::npos || exe_lower.find("cl ") != std::string::npos || exe_lower == "cl") {
        info.family = CompilerFamily::MSVC;
    }

    for (size_t i = 1; i < args.size(); ++i) {
        std::string arg = utils::trim(args[i]);
        if (arg.empty()) continue;

        // Language standard: -std=c++20, /std:c++20
        if (utils::starts_with(arg, "-std=") || utils::starts_with(arg, "--std=")) {
            info.language_standard = arg.substr(arg.find('=') + 1);
        } else if (utils::starts_with(arg, "/std:")) {
            info.language_standard = arg.substr(5);
        }

        // Optimization level: -O0, -O1, -O2, -O3, -Os, -Oz, -Ofast, /O2, /Od, /O1, /Ox
        else if (utils::starts_with(arg, "-O") || utils::starts_with(arg, "/O")) {
            info.optimization_level = arg.substr(2);
        }

        // Debug symbols: -g, -g3, /Zi, /Z7, /ZI
        else if (arg == "-g" || utils::starts_with(arg, "-g") || arg == "/Zi" || arg == "/Z7" || arg == "/ZI") {
            info.has_debug_symbols = true;
        }

        // Warning flags: -Wall, -Wextra, -Wpedantic, -Werror, /W4, /WX
        else if (utils::starts_with(arg, "-W") || utils::starts_with(arg, "/W")) {
            info.warning_flags.push_back(arg);
        }

        // System include directories: -isystem, /imsvc
        else if (arg == "-isystem" || arg == "/imsvc") {
            if (i + 1 < args.size()) {
                info.system_include_dirs.push_back(args[++i]);
            }
        }
        // Include directories: -I, /I, -iquote
        else if (utils::starts_with(arg, "-I") || utils::starts_with(arg, "/I")) {
            std::string inc = arg.substr(2);
            if (inc.empty() && i + 1 < args.size()) inc = args[++i];
            if (!inc.empty()) info.include_dirs.push_back(inc);
        }

        // Defines: -D, /D
        else if (utils::starts_with(arg, "-D") || utils::starts_with(arg, "/D")) {
            std::string def = arg.substr(2);
            if (def.empty() && i + 1 < args.size()) def = args[++i];
            if (!def.empty()) info.defines.push_back(def);
        }

        // Forced includes: -include, /FI
        else if (arg == "-include" || arg == "/FI") {
            if (i + 1 < args.size()) info.forced_includes.push_back(args[++i]);
        }

        // LTO: -flto, /GL
        else if (arg == "-flto" || arg == "/GL") {
            info.has_lto = true;
        }

        // Sanitizers: -fsanitize=...
        else if (utils::starts_with(arg, "-fsanitize=")) {
            info.has_sanitizers = true;
        }

        // Target architecture: -m64, -m32, -march=...
        else if (utils::starts_with(arg, "-m") || utils::starts_with(arg, "/arch:")) {
            info.target_architecture = arg;
        }
    }

    return info;
}

} // namespace compileforge
