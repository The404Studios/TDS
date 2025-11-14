#include "AuthManager.h"
#include "../Server.h"
#include <iostream>

namespace TDS {

AuthManager::AuthManager(Server* server) : server(server) {
    std::cout << "[AuthManager] Initialized" << std::endl;
}

AuthManager::~AuthManager() {}

void AuthManager::update(float dt) {
    // TODO: Implement manager update logic
}

void AuthManager::saveAllPlayers() {
    // TODO: Save player data to database
}

} // namespace TDS
