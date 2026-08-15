#include <monolith_app/include/core/common.hpp>
#include <monolith_app/include/network/types.hpp>

namespace monolith::network {

void send_packet(const Packet& p) {
    monolith::core::log_common("Sending packet ID: " + std::to_string(p.id));
}

} // namespace monolith::network
