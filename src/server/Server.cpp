#include "Server.h"
#include "Network/NetworkServer.h"
#include "Database/Database.h"
#include "Managers/AuthManager.h"
#include "Managers/PlayerManager.h"
#include "Managers/MatchManager.h"
#include "Managers/LootManager.h"
#include "Managers/MerchantManager.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace TDS {

Server::Server() : running(false), port(0) {}

Server::~Server() {
    shutdown();
}

bool Server::initialize(uint16_t p) {
    port = p;

    std::cout << "[Server] Initializing subsystems..." << std::endl;

    // Initialize database
    database = std::make_unique<Database>();
    if (!database->initialize("tds_server.db")) {
        std::cerr << "[Server] Failed to initialize database" << std::endl;
        return false;
    }

    // Initialize managers
    authManager = std::make_unique<AuthManager>(this);
    playerManager = std::make_unique<PlayerManager>(this);
    matchManager = std::make_unique<MatchManager>(this);
    lootManager = std::make_unique<LootManager>(this);
    merchantManager = std::make_unique<MerchantManager>(this);

    std::cout << "[Server] Managers initialized" << std::endl;

    // Initialize network server (must be last)
    network = std::make_unique<NetworkServer>(this);
    if (!network->start(port)) {
        std::cerr << "[Server] Failed to start network server" << std::endl;
        return false;
    }

    std::cout << "[Server] Initialized on port " << port << std::endl;
    running = true;
    return true;
}

void Server::run() {
    auto lastTime = std::chrono::steady_clock::now();
    const float targetDt = 1.0f / 60.0f;  // 60Hz tick rate

    std::cout << "[Server] Starting main loop (60Hz tick rate)" << std::endl;

    while (running) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        update(dt);

        // Sleep to maintain 60Hz tick rate
        auto frameDuration = std::chrono::steady_clock::now() - now;
        auto targetFrameDuration = std::chrono::duration<float>(targetDt);
        if (frameDuration < targetFrameDuration) {
            std::this_thread::sleep_for(targetFrameDuration - frameDuration);
        }
    }

    std::cout << "[Server] Main loop ended" << std::endl;
}

void Server::update(float dt) {
    // Process network packets
    if (network) {
        network->update();
    }

    // Update managers
    if (authManager) {
        authManager->update(dt);
    }
    if (playerManager) {
        playerManager->update(dt);
    }
    if (matchManager) {
        matchManager->update(dt);
    }
    if (lootManager) {
        lootManager->update(dt);
    }
    if (merchantManager) {
        merchantManager->update(dt);
    }
}

void Server::shutdown() {
    if (!running) return;

    running = false;

    std::cout << "[Server] Shutting down..." << std::endl;

    // Shutdown in reverse order
    if (network) {
        network->stop();
        network.reset();
    }

    // Managers
    merchantManager.reset();
    lootManager.reset();
    matchManager.reset();
    playerManager.reset();
    authManager.reset();

    // Database last
    database.reset();

    std::cout << "[Server] Shutdown complete" << std::endl;
}

} // namespace TDS
