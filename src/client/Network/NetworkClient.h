#pragma once
#include <string>
#include <cstdint>

namespace TDS {
class Game;

class NetworkClient {
public:
    NetworkClient(Game* game);
    ~NetworkClient();

    bool connect(const std::string& host, uint16_t port);
    void disconnect();
    void update();

    bool isConnected() const { return connected; }

private:
    Game* game;
    bool connected;
};
}
