#include <compileforge/config/config.hpp>
#include <compileforge/core/json.hpp>
#include <fstream>

namespace compileforge {

Config Config::default_config() {
    Config cfg;
    cfg.exclude_paths = {"build", ".git", "third_party", "vendor"};
    cfg.max_hotspot_score_threshold = 80.0;
    cfg.max_allowed_cycles = 0;
    cfg.fail_on_cycles = true;
    return cfg;
}

Result<Config> Config::load_file(const std::string& config_filepath) {
    std::ifstream ifs(config_filepath);
    if (!ifs) {
        return default_config();
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto parse_res = JsonValue::parse(content);
    if (parse_res.is_error()) return parse_res.error();

    Config cfg = default_config();
    const auto& root = parse_res.value();

    if (root["exclude_paths"].is_array()) {
        cfg.exclude_paths.clear();
        for (const auto& item : root["exclude_paths"].as_array()) {
            cfg.exclude_paths.push_back(item.as_string());
        }
    }
    if (root["custom_include_dirs"].is_array()) {
        for (const auto& item : root["custom_include_dirs"].as_array()) {
            cfg.custom_include_dirs.push_back(item.as_string());
        }
    }
    if (root.contains("max_hotspot_score_threshold")) {
        cfg.max_hotspot_score_threshold = root["max_hotspot_score_threshold"].as_double(80.0);
    }
    if (root.contains("fail_on_cycles")) {
        cfg.fail_on_cycles = root["fail_on_cycles"].as_bool(true);
    }

    return cfg;
}

} // namespace compileforge
