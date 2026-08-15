#ifndef COMPILEFORGE_CONFIG_CONFIG_HPP
#define COMPILEFORGE_CONFIG_CONFIG_HPP

#include <string>
#include <vector>
#include <compileforge/core/result.hpp>

namespace compileforge {

struct Config {
    std::vector<std::string> ignore_patterns;
    std::vector<std::string> custom_include_dirs;
    size_t complexity_threshold{15};
    size_t fan_in_threshold{20};
    size_t fan_out_threshold{15};
    size_t dependency_depth_threshold{10};
    double hotspot_score_threshold{80.0};
    size_t churn_threshold{10};
    double build_duration_threshold_seconds{5.0};
    size_t include_count_threshold{25};

    static Result<Config> load_file(const std::string& config_filepath);
    static Config default_config();
    static bool save_config(const std::string& config_filepath, const Config& config);
};

} // namespace compileforge

#endif // COMPILEFORGE_CONFIG_CONFIG_HPP
