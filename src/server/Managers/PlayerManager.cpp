#include "PlayerManager.h"
#include "../Server.h"
#include <iostream>

namespace TDS {

PlayerManager::PlayerManager(Server* server) : server(server) {
    std::cout << "[PlayerManager] Initialized" << std::endl;
}

PlayerManager::~PlayerManager() {}

void PlayerManager::update(float dt) {
    // TODO: Implement manager update logic
}

void PlayerManager::saveAllPlayers() {
    // TODO: Save player data to database
}

} // namespace TDS
