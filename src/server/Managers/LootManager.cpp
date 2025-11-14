#include "LootManager.h"
#include "../Server.h"
#include <iostream>

namespace TDS {

LootManager::LootManager(Server* server) : server(server) {
    std::cout << "[LootManager] Initialized" << std::endl;
}

LootManager::~LootManager() {}

void LootManager::update(float dt) {
    // TODO: Implement manager update logic
}

void LootManager::saveAllPlayers() {
    // TODO: Save player data to database
}

} // namespace TDS
