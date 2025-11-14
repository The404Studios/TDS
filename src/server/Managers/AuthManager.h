#pragma once

namespace TDS {

class Server;

class AuthManager {
public:
    AuthManager(Server* server);
    ~AuthManager();
    
    void update(float dt);
    void saveAllPlayers();

private:
    Server* server;
};

} // namespace TDS
