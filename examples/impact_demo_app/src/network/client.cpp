#include "../../include/network/client.hpp"
#include <iostream>

namespace demo {
    void Client::send(const Message& msg) {
        std::cout << "Sending msg: " << msg.payload << "\n";
    }
}
