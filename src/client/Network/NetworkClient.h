#pragma once

#include <string>
#include <cstdint>
#include "common/Protocol.h"

// Forward declare ENet types to avoid including enet.h in header
typedef struct _ENetHost ENetHost;
typedef struct _ENetPeer ENetPeer;
typedef struct _ENetPacket ENetPacket;

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
    uint32_t getPlayerId() const { return playerId; }
    uint64_t getSessionToken() const { return sessionToken; }

    // Send packets
    void sendPacket(const uint8_t* data, size_t size, ENetChannel channel);
    void sendPlayerMove(const PlayerState& state);
    void sendWeaponFire(const WeaponFireEvent& fireEvent);
    void sendLoginRequest(const std::string& username, const std::string& password);

private:
    void handlePacket(ENetPacket* packet);
    void handleLoginResponse(PacketReader& reader);
    void handlePlayerJoin(PacketReader& reader);
    void handlePlayerMove(PacketReader& reader);
    void handleDamage(PacketReader& reader);
    void handleWeaponShoot(PacketReader& reader);
    void handlePing(PacketReader& reader);

    Game* game;
    bool connected;

    // ENet
    ENetHost* host;
    ENetPeer* peer;

    // Player data
    uint32_t playerId;
    uint64_t sessionToken;
};

} // namespace TDS
