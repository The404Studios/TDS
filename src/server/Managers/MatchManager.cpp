#include "MatchManager.h"
#include "../Server.h"
#include <iostream>

namespace TDS {

MatchManager::MatchManager(Server* server) : server(server) {
    std::cout << "[MatchManager] Initialized" << std::endl;
}

MatchManager::~MatchManager() {}

void MatchManager::update(float dt) {
    // TODO: Implement manager update logic
}

void MatchManager::saveAllPlayers() {
    // TODO: Save player data to database
}

} // namespace TDS
