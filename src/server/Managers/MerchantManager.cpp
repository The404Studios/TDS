#include "MerchantManager.h"
#include "../Server.h"
#include <iostream>

namespace TDS {

MerchantManager::MerchantManager(Server* server) : server(server) {
    std::cout << "[MerchantManager] Initialized" << std::endl;
}

MerchantManager::~MerchantManager() {}

void MerchantManager::update(float dt) {
    // TODO: Implement manager update logic
}

void MerchantManager::saveAllPlayers() {
    // TODO: Save player data to database
}

} // namespace TDS
