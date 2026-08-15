#include "../../include/compileforge/config/config.hpp"
#include "../../include/compileforge/core/json.hpp"
#include <fstream>

namespace compileforge {

Config Config::default_config() {
    Config cfg;
    cfg.ignore_patterns = {"build", ".git", "out", "target", "node_modules"};
    return cfg;
}

Result<Config> Config::load_file(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs) {
        return default_config();
    }

    std::string content((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    auto parse_res = JsonValue::parse(content);
    if (parse_res.is_error()) {
        return default_config();
    }

    Config cfg = default_config();
    const auto& root = parse_res.value();

    if (root["fan_in_threshold"].is_number()) {
        cfg.fan_in_threshold = static_cast<size_t>(root["fan_in_threshold"].as_int());
    }
    if (root["fan_out_threshold"].is_number()) {
        cfg.fan_out_threshold = static_cast<size_t>(root["fan_out_threshold"].as_int());
    }
    if (root["hotspot_score_threshold"].is_number()) {
        cfg.hotspot_score_threshold = root["hotspot_score_threshold"].as_double();
    }

    return cfg;
}

bool Config::save_config(const std::string& filepath, const Config& cfg) {
    JsonValue::ObjectType root;
    root["fan_in_threshold"] = static_cast<int>(cfg.fan_in_threshold);
    root["fan_out_threshold"] = static_cast<int>(cfg.fan_out_threshold);
    root["hotspot_score_threshold"] = cfg.hotspot_score_threshold;

    JsonValue::ArrayType ign_arr;
    for (const auto& ign : cfg.ignore_patterns) ign_arr.push_back(ign);
    root["ignore_patterns"] = ign_arr;

    std::ofstream ofs(filepath);
    if (!ofs) return false;
    ofs << JsonValue(root).serialize(2);
    return true;
}

} // namespace compileforge
