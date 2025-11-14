#pragma once

namespace TDS {

class Server;

class LootManager {
public:
    LootManager(Server* server);
    ~LootManager();
    
    void update(float dt);
    void saveAllPlayers();

private:
    Server* server;
};

} // namespace TDS
