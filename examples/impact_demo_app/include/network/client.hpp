#pragma once
#include "../core/types.hpp"

namespace demo {
    class Client {
    public:
        void send(const Message& msg);
    };
}
