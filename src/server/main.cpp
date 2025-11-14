#include "Server.h"
#include <iostream>
#include <csignal>

TDS::Server* g_server = nullptr;

void signalHandler(int signal) {
    std::cout << "\n[Main] Received signal " << signal << std::endl;
    if (g_server) {
        g_server->shutdown();
    }
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "  TDS Server - Tarkov Looter Shooter\n";
    std::cout << "===========================================\n";
    std::cout << std::endl;

    // Setup signal handlers
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    TDS::Server server;
    g_server = &server;

    if (!server.initialize(7777)) {
        std::cerr << "Failed to initialize server!\n";
        return 1;
    }

    std::cout << "Server initialized successfully!\n";
    std::cout << "Listening on port 7777\n";
    std::cout << "Press Ctrl+C to stop\n";
    std::cout << std::endl;

    server.run();

    return 0;
}
