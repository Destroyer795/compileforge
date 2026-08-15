#ifndef COMPILEFORGE_CONFIG_CONFIG_HPP
#define COMPILEFORGE_CONFIG_CONFIG_HPP

#include <string>
#include <vector>
#include <compileforge/core/result.hpp>

namespace compileforge {

struct Config {
    std::vector<std::string> exclude_paths;
    std::vector<std::string> custom_include_dirs;
    double max_hotspot_score_threshold{80.0};
    size_t max_allowed_cycles{0};
    bool fail_on_cycles{true};

    static Result<Config> load_file(const std::string& config_filepath);
    static Config default_config();
};

} // namespace compileforge

#endif // COMPILEFORGE_CONFIG_CONFIG_HPP
