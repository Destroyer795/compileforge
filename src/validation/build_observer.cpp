#include <compileforge/validation/build_observer.hpp>
#include <compileforge/core/utils.hpp>
#include <fstream>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <regex>

namespace compileforge {

std::vector<std::string> BuildObserver::extract_compiled_sources_from_log(const std::string& log_text) {
    std::vector<std::string> sources;
    std::istringstream iss(log_text);
    std::string line;

    while (std::getline(iss, line)) {
        std::string trimmed = utils::trim(line);
        if (trimmed.empty()) continue;

        // Ninja pattern: [X/Y] Building CXX object ... /path/to/file.cpp.o
        size_t cxx_obj = trimmed.find("Building CXX object ");
        if (cxx_obj != std::string::npos) {
            size_t src_pos = trimmed.find(".dir/");
            if (src_pos != std::string::npos) {
                std::string sub = trimmed.substr(src_pos + 5);
                if (utils::ends_with(sub, ".o") || utils::ends_with(sub, ".obj")) {
                    sub = sub.substr(0, sub.find_last_of('.'));
                }
                sources.push_back(utils::normalize_path(sub));
                continue;
            }
        }

        // Direct compiler command pattern: g++ ... -c src/foo.cpp -o ...
        size_t c_flag = trimmed.find(" -c ");
        if (c_flag != std::string::npos) {
            auto tokens = utils::split(trimmed, ' ');
            for (size_t i = 0; i < tokens.size(); ++i) {
                if (tokens[i] == "-c" && i + 1 < tokens.size()) {
                    std::string src = utils::trim(tokens[i + 1]);
                    if (!src.empty() && src[0] != '-') {
                        sources.push_back(utils::normalize_path(src));
                    }
                }
            }
        }
    }

    // Sort and deduplicate
    std::sort(sources.begin(), sources.end());
    sources.erase(std::unique(sources.begin(), sources.end()), sources.end());
    return sources;
}

Result<BuildObservation> BuildObserver::parse_build_log(const std::string& log_or_path) {
    std::string content = log_or_path;
    if (std::filesystem::exists(log_or_path)) {
        std::ifstream ifs(log_or_path);
        if (ifs) {
            content.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        }
    }

    BuildObservation obs;
    obs.rebuilt_tus = extract_compiled_sources_from_log(content);
    obs.total_compiler_invocations = obs.rebuilt_tus.size();
    obs.observation_source = "BUILD_LOG";
    obs.has_measured_duration = false;
    obs.actual_build_duration_seconds = 0.0;
    return obs;
}

Result<BuildObservation> BuildObserver::observe_command(const std::string& build_cmd, const std::string& working_dir) {
    auto start = std::chrono::high_resolution_clock::now();
    int exit_code = 0;
    std::string full_cmd = "cd \"" + working_dir + "\" && " + build_cmd;
    std::string output = utils::execute_command(full_cmd, &exit_code);
    auto end = std::chrono::high_resolution_clock::now();

    double duration_s = std::chrono::duration<double>(end - start).count();

    BuildObservation obs;
    obs.rebuilt_tus = extract_compiled_sources_from_log(output);
    obs.total_compiler_invocations = obs.rebuilt_tus.size();
    obs.observation_source = "BUILD_COMMAND";
    obs.has_measured_duration = true;
    obs.actual_build_duration_seconds = duration_s;
    return obs;
}

} // namespace compileforge
