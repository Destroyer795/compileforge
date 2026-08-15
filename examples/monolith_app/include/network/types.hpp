#pragma once
#include "../core/common.hpp"
#include <cstdint>
#include <string>

namespace monolith::network {

struct Packet {
    uint32_t id;
    std::string payload;
};

} // namespace monolith::network
