#include "Game.h"
#include "raylib.h"
#include <iostream>
#include <memory>

using namespace TDS;

int main(int argc, char* argv[]) {
    // Set trace log level
    SetTraceLogLevel(LOG_INFO);

    std::cout << "===========================================\n";
    std::cout << "  TDS - Tarkov Looter Shooter (Raylib)\n";
    std::cout << "===========================================\n";
    std::cout << "Initializing...\n";

    // Create game instance
    auto game = std::make_unique<Game>();

    // Initialize
    if (!game->initialize()) {
        std::cerr << "Failed to initialize game!" << std::endl;
        return 1;
    }

    std::cout << "Game initialized successfully!\n";
    std::cout << "Starting main loop...\n";

    // Run game loop
    game->run();

    // Cleanup
    std::cout << "Shutting down...\n";
    game->shutdown();

    std::cout << "Goodbye!\n";
    return 0;
}
