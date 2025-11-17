#pragma once
#include <cstdint>
#include <memory>

namespace TDS {

// Forward declarations
class NetworkServer;
class Database;
class AuthManager;
class PlayerManager;
class MatchManager;
class LootManager;
class MerchantManager;

class Server {
public:
    Server();
    ~Server();

    bool initialize(uint16_t port);
    void run();
    void shutdown();

    // Accessors for subsystems
    NetworkServer* getNetwork() { return network.get(); }
    Database* getDatabase() { return database.get(); }
    AuthManager* getAuthManager() { return authManager.get(); }
    PlayerManager* getPlayerManager() { return playerManager.get(); }
    MatchManager* getMatchManager() { return matchManager.get(); }
    LootManager* getLootManager() { return lootManager.get(); }
    MerchantManager* getMerchantManager() { return merchantManager.get(); }

private:
    void update(float dt);

    bool running;
    uint16_t port;

    // Subsystems
    std::unique_ptr<NetworkServer> network;
    std::unique_ptr<Database> database;
    std::unique_ptr<AuthManager> authManager;
    std::unique_ptr<PlayerManager> playerManager;
    std::unique_ptr<MatchManager> matchManager;
    std::unique_ptr<LootManager> lootManager;
    std::unique_ptr<MerchantManager> merchantManager;
};

} // namespace TDS
