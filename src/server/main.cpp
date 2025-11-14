#include "Server.h"
#include <iostream>

int main() {
    std::cout << "===========================================\n";
    std::cout << "  TDS Server - Tarkov Looter Shooter\n";
    std::cout << "===========================================\n";

    TDS::Server server;

    if (!server.initialize(7777)) {
        std::cerr << "Failed to initialize server!\n";
        return 1;
    }

    std::cout << "Server running on port 7777\n";
    std::cout << "Press Ctrl+C to stop\n";

    server.run();

    return 0;
}
