#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace monolith::core {

struct AppConfig {
    std::string app_name = "MonolithApp";
    int max_threads = 8;
    bool debug_mode = true;
};

template <typename T>
class HeavyweightContainer {
public:
    void add(const T& item) { items_.push_back(item); }
    size_t size() const { return items_.size(); }
private:
    std::vector<T> items_;
};

inline void log_common(const std::string& msg) {
    std::cout << "[LOG] " << msg << std::endl;
}

} // namespace monolith::core
