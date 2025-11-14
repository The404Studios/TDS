#include "NetworkClient.h"
#include "../Game.h"
#include "common/Protocol.h"
#include <enet/enet.h>
#include <cstring>

namespace TDS {

NetworkClient::NetworkClient(Game* game)
    : game(game)
    , connected(false)
    , host(nullptr)
    , peer(nullptr)
    , playerId(0)
    , sessionToken(0)
{
    // Initialize ENet
    if (enet_initialize() != 0) {
        TraceLog(LOG_ERROR, "Failed to initialize ENet");
    } else {
        TraceLog(LOG_INFO, "ENet initialized successfully");
    }
}

NetworkClient::~NetworkClient() {
    disconnect();
    enet_deinitialize();
}

bool NetworkClient::connect(const std::string& hostname, uint16_t port) {
    if (connected) {
        TraceLog(LOG_WARNING, "Already connected to server");
        return true;
    }

    // Create ENet host for client
    host = enet_host_create(
        nullptr,        // Create a client host
        1,              // Only allow 1 outgoing connection
        3,              // 3 channels (reliable, unreliable, unordered)
        0,              // No download bandwidth limit
        0               // No upload bandwidth limit
    );

    if (host == nullptr) {
        TraceLog(LOG_ERROR, "Failed to create ENet client host");
        return false;
    }

    // Set up server address
    ENetAddress address;
    enet_address_set_host(&address, hostname.c_str());
    address.port = port;

    // Connect to server
    peer = enet_host_connect(host, &address, 3, 0);

    if (peer == nullptr) {
        TraceLog(LOG_ERROR, "No available peers for initiating connection");
        enet_host_destroy(host);
        host = nullptr;
        return false;
    }

    // Wait for connection (timeout: 5 seconds)
    ENetEvent event;
    if (enet_host_service(host, &event, 5000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        TraceLog(LOG_INFO, "Connected to server at %s:%d", hostname.c_str(), port);
        connected = true;
        return true;
    } else {
        TraceLog(LOG_ERROR, "Connection to %s:%d failed", hostname.c_str(), port);
        enet_peer_reset(peer);
        enet_host_destroy(host);
        host = nullptr;
        peer = nullptr;
        return false;
    }
}

void NetworkClient::disconnect() {
    if (!connected || !peer) return;

    // Send disconnect packet
    ENetEvent event;
    enet_peer_disconnect(peer, 0);

    // Wait for disconnect confirmation
    while (enet_host_service(host, &event, 3000) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                TraceLog(LOG_INFO, "Disconnected from server");
                connected = false;
                break;
            default:
                break;
        }
    }

    if (connected) {
        // Force disconnect
        enet_peer_reset(peer);
        connected = false;
    }

    if (host) {
        enet_host_destroy(host);
        host = nullptr;
    }
    peer = nullptr;
}

void NetworkClient::update() {
    if (!connected || !host) return;

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                handlePacket(event.packet);
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                TraceLog(LOG_WARNING, "Server disconnected");
                connected = false;
                peer = nullptr;
                break;

            default:
                break;
        }
    }
}

void NetworkClient::sendPacket(const uint8_t* data, size_t size, ENetChannel channel) {
    if (!connected || !peer) return;

    enet_uint32 flags = 0;
    switch (channel) {
        case ENetChannel::RELIABLE_ORDERED:
            flags = ENET_PACKET_FLAG_RELIABLE;
            break;
        case ENetChannel::UNRELIABLE_SEQUENCED:
            flags = ENET_PACKET_FLAG_UNSEQUENCED;
            break;
        case ENetChannel::RELIABLE_UNORDERED:
            flags = ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_UNSEQUENCED;
            break;
    }

    ENetPacket* packet = enet_packet_create(data, size, flags);
    enet_peer_send(peer, static_cast<enet_uint8>(channel), packet);
}

void NetworkClient::sendPlayerMove(const PlayerState& state) {
    PacketBuilder builder(PacketType::PLAYER_MOVE);
    builder.writePlayerState(state);
    sendPacket(builder.getData(), builder.getSize(), ENetChannel::UNRELIABLE_SEQUENCED);
}

void NetworkClient::sendWeaponFire(const WeaponFireEvent& fireEvent) {
    PacketBuilder builder(PacketType::WEAPON_SHOOT);
    builder.writeUInt32(fireEvent.playerId);
    builder.writeVector3(fireEvent.origin);
    builder.writeVector3(fireEvent.direction);
    builder.writeUInt16(fireEvent.weaponId);
    builder.writeUInt32(fireEvent.timestamp);
    sendPacket(builder.getData(), builder.getSize(), ENetChannel::UNRELIABLE_SEQUENCED);
}

void NetworkClient::sendLoginRequest(const std::string& username, const std::string& password) {
    PacketBuilder builder(PacketType::AUTH_LOGIN_REQUEST);
    builder.writeString(username);
    builder.writeString(password);
    sendPacket(builder.getData(), builder.getSize(), ENetChannel::RELIABLE_ORDERED);
}

void NetworkClient::handlePacket(ENetPacket* packet) {
    if (packet->dataLength < 1) return;

    PacketReader reader(packet->data, packet->dataLength);
    PacketType type = reader.getType();

    // Skip the type byte
    reader.readUInt8();

    switch (type) {
        case PacketType::AUTH_LOGIN_RESPONSE:
            handleLoginResponse(reader);
            break;

        case PacketType::MATCH_PLAYER_JOIN:
            handlePlayerJoin(reader);
            break;

        case PacketType::PLAYER_MOVE:
            handlePlayerMove(reader);
            break;

        case PacketType::DAMAGE_DEALT:
            handleDamage(reader);
            break;

        case PacketType::WEAPON_SHOOT:
            handleWeaponShoot(reader);
            break;

        case PacketType::PING:
            handlePing(reader);
            break;

        default:
            TraceLog(LOG_WARNING, "Unhandled packet type: %d", (int)type);
            break;
    }
}

void NetworkClient::handleLoginResponse(PacketReader& reader) {
    bool success = reader.readUInt8() != 0;

    if (success) {
        playerId = reader.readUInt32();
        sessionToken = reader.readUInt64();

        game->setPlayerId(playerId);
        game->setSessionToken(sessionToken);

        TraceLog(LOG_INFO, "Login successful! Player ID: %u", playerId);
        game->setState(GameState::MAIN_MENU);
    } else {
        std::string errorMsg = reader.readString();
        TraceLog(LOG_ERROR, "Login failed: %s", errorMsg.c_str());
    }
}

void NetworkClient::handlePlayerJoin(PacketReader& reader) {
    uint32_t newPlayerId = reader.readUInt32();
    std::string playerName = reader.readString();

    TraceLog(LOG_INFO, "Player joined: %s (ID: %u)", playerName.c_str(), newPlayerId);

    // TODO: Create remote player object
}

void NetworkClient::handlePlayerMove(PacketReader& reader) {
    PlayerState state = reader.readPlayerState();

    // TODO: Update remote player position
    // game->updateRemotePlayer(state);
}

void NetworkClient::handleDamage(PacketReader& reader) {
    uint32_t victimId = reader.readUInt32();
    uint16_t damage = reader.readUInt16();

    if (victimId == playerId) {
        TraceLog(LOG_INFO, "Took %d damage!", damage);
        // TODO: Update player health
        // game->getPlayer()->takeDamage(damage);
    }
}

void NetworkClient::handleWeaponShoot(PacketReader& reader) {
    uint32_t shooterId = reader.readUInt32();
    Vector3 origin = reader.readVector3();
    Vector3 direction = reader.readVector3();

    // TODO: Play weapon fire effect for remote player
}

void NetworkClient::handlePing(PacketReader& reader) {
    uint32_t timestamp = reader.readUInt32();

    // Send pong
    PacketBuilder builder(PacketType::PONG);
    builder.writeUInt32(timestamp);
    sendPacket(builder.getData(), builder.getSize(), ENetChannel::RELIABLE_UNORDERED);
}

}
