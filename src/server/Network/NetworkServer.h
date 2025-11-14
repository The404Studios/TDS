#pragma once

#include <cstdint>
#include <map>
#include "common/Protocol.h"

typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;
typedef struct _ENetPacket ENetPacket;

namespace TDS {

class Server;

struct ClientInfo {
    ENetPeer* peer;
    uint32_t clientId;
    uint32_t playerId;
    bool authenticated;
};

class NetworkServer {
public:
    NetworkServer(Server* server);
    ~NetworkServer();

    bool start(uint16_t port);
    void stop();
    void update();

    void sendPacket(ENetPeer* peer, const uint8_t* data, size_t size, ENetChannel channel);
    void broadcast(const uint8_t* data, size_t size, ENetChannel channel);

private:
    void handleConnect(ENetPeer* peer);
    void handleDisconnect(ENetPeer* peer);
    void handlePacket(ENetPeer* peer, ENetPacket* packet);

    void handleLogin(ENetPeer* peer, PacketReader& reader);
    void handleRegister(ENetPeer* peer, PacketReader& reader);
    void handlePlayerMove(ENetPeer* peer, PacketReader& reader);
    void handleWeaponShoot(ENetPeer* peer, PacketReader& reader);

    Server* server;
    ENetHost* host;
    std::map<uint32_t, ClientInfo> clients;
    uint32_t nextClientId = 1;
};

} // namespace TDS
