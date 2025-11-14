#pragma once

namespace TDS {

class Server;

class MerchantManager {
public:
    MerchantManager(Server* server);
    ~MerchantManager();
    
    void update(float dt);
    void saveAllPlayers();

private:
    Server* server;
};

} // namespace TDS
