#include "NetworkClient.h"
#include "../Game.h"
#include "raylib.h"

namespace TDS {

NetworkClient::NetworkClient(Game* game) : game(game), connected(false) {}
NetworkClient::~NetworkClient() { disconnect(); }

bool NetworkClient::connect(const std::string& host, uint16_t port) {
    TraceLog(LOG_INFO, "NetworkClient: Connecting to %s:%d", host.c_str(), port);
    // TODO: Implement ENet connection
    connected = true;
    return true;
}

void NetworkClient::disconnect() {
    if (connected) {
        TraceLog(LOG_INFO, "NetworkClient: Disconnecting");
        connected = false;
    }
}

void NetworkClient::update() {
    if (!connected) return;
    // TODO: Process incoming packets
}

}
