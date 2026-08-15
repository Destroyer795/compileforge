#include "../../include/core/common.hpp"
#include "../../include/network/types.hpp"
#include <vector>
#include <iostream>

namespace monolith::render {

void render_scene() {
    monolith::core::log_common("Rendering 3D viewport scene...");
    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            // heavy render loop simulation
        }
    }
}

} // namespace monolith::render
