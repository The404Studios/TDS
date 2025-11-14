#pragma once

namespace TDS {

class Server;

class PlayerManager {
public:
    PlayerManager(Server* server);
    ~PlayerManager();
    
    void update(float dt);
    void saveAllPlayers();

private:
    Server* server;
};

} // namespace TDS
