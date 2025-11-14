#pragma once

namespace TDS {

class Server;

class MatchManager {
public:
    MatchManager(Server* server);
    ~MatchManager();
    
    void update(float dt);
    void saveAllPlayers();

private:
    Server* server;
};

} // namespace TDS
