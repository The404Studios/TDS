// NAT Punchthrough Server - Main Entry Point
// Standalone server that facilitates P2P connections

#include "NatPunchServer.h"
#include <iostream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

bool g_running = true;

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT) {
        std::cout << "\n[NatServer] Shutdown signal received..." << std::endl;
        g_running = false;
        return TRUE;
    }
    return FALSE;
}
#endif

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  NAT Punchthrough Server" << std::endl;
    std::cout << "  UDP Hole Punching Facilitator" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Parse command line arguments
    uint16_t port = 3478; // Default STUN port
    if (argc > 1) {
        port = static_cast<uint16_t>(atoi(argv[1]));
    }

#ifdef _WIN32
    // Set up console handler for graceful shutdown
    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
#endif

    // Create and start NAT server
    NatPunchServer server(port);

    if (!server.start()) {
        std::cout << "[NatServer] Failed to start server!" << std::endl;
        return 1;
    }

    std::cout << "[NatServer] Server is running..." << std::endl;
    std::cout << "[NatServer] Press Ctrl+C to stop" << std::endl;
    std::cout << std::endl;

    // Main server loop
    auto lastStatsTime = std::chrono::steady_clock::now();

    while (g_running && server.isRunning()) {
        // Update server
        server.update();

        // Print stats every 10 seconds
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastStatsTime).count();
        if (elapsed >= 10) {
            std::cout << "[NatServer] Active clients: " << server.getClientCount() << std::endl;
            lastStatsTime = now;
        }

        // Small sleep to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Cleanup
    std::cout << "[NatServer] Shutting down..." << std::endl;
    server.stop();
    std::cout << "[NatServer] Shutdown complete" << std::endl;

    return 0;
}
