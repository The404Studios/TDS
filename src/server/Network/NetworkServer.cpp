#include "NetworkServer.h"
#include "../Server.h"
#include "common/Protocol.h"
#include <enet/enet.h>
#include <iostream>

namespace TDS {

NetworkServer::NetworkServer(Server* server)
    : server(server)
    , host(nullptr)
{
    if (enet_initialize() != 0) {
        std::cerr << "[NetworkServer] Failed to initialize ENet!" << std::endl;
    } else {
        std::cout << "[NetworkServer] ENet initialized" << std::endl;
    }
}

NetworkServer::~NetworkServer() {
    stop();
    enet_deinitialize();
}

bool NetworkServer::start(uint16_t port) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    // Create ENet host
    host = enet_host_create(
        &address,       // Address to bind
        32,             // Max clients
        3,              // 3 channels
        0,              // No download bandwidth limit
        0               // No upload bandwidth limit
    );

    if (host == nullptr) {
        std::cerr << "[NetworkServer] Failed to create ENet host!" << std::endl;
        return false;
    }

    std::cout << "[NetworkServer] Server started on port " << port << std::endl;
    return true;
}

void NetworkServer::stop() {
    if (!host) return;

    // Disconnect all clients
    for (auto& pair : clients) {
        enet_peer_disconnect(pair.second.peer, 0);
    }

    // Process remaining events
    ENetEvent event;
    while (enet_host_service(host, &event, 100) > 0) {
        if (event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
        }
    }

    enet_host_destroy(host);
    host = nullptr;
    clients.clear();

    std::cout << "[NetworkServer] Server stopped" << std::endl;
}

void NetworkServer::update() {
    if (!host) return;

    ENetEvent event;
    while (enet_host_service(host, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT:
                handleConnect(event.peer);
                break;

            case ENET_EVENT_TYPE_RECEIVE:
                handlePacket(event.peer, event.packet);
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                handleDisconnect(event.peer);
                break;

            default:
                break;
        }
    }
}

void NetworkServer::sendPacket(ENetPeer* peer, const uint8_t* data, size_t size, ENetChannel channel) {
    if (!peer) return;

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

void NetworkServer::broadcast(const uint8_t* data, size_t size, ENetChannel channel) {
    if (!host) return;

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
    enet_host_broadcast(host, static_cast<enet_uint8>(channel), packet);
}

void NetworkServer::handleConnect(ENetPeer* peer) {
    uint32_t clientId = nextClientId++;

    ClientInfo info;
    info.peer = peer;
    info.clientId = clientId;
    info.playerId = 0;
    info.authenticated = false;

    clients[clientId] = info;
    peer->data = reinterpret_cast<void*>(static_cast<uintptr_t>(clientId));

    std::cout << "[NetworkServer] Client connected: " << clientId << std::endl;
}

void NetworkServer::handleDisconnect(ENetPeer* peer) {
    if (!peer->data) return;

    uint32_t clientId = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(peer->data));
    
    auto it = clients.find(clientId);
    if (it != clients.end()) {
        std::cout << "[NetworkServer] Client disconnected: " << clientId << std::endl;
        
        // Notify other systems
        if (it->second.authenticated && it->second.playerId != 0) {
            // TODO: Notify match manager, etc.
        }

        clients.erase(it);
    }
}

void NetworkServer::handlePacket(ENetPeer* peer, ENetPacket* packet) {
    if (packet->dataLength < 1) return;

    PacketReader reader(packet->data, packet->dataLength);
    PacketType type = reader.getType();

    // Skip type byte
    reader.readUInt8();

    // Route packet to appropriate handler
    switch (type) {
        case PacketType::AUTH_LOGIN_REQUEST:
            handleLogin(peer, reader);
            break;

        case PacketType::AUTH_REGISTER_REQUEST:
            handleRegister(peer, reader);
            break;

        case PacketType::PLAYER_MOVE:
            handlePlayerMove(peer, reader);
            break;

        case PacketType::WEAPON_SHOOT:
            handleWeaponShoot(peer, reader);
            break;

        case PacketType::PONG:
            // Latency measurement
            break;

        default:
            std::cout << "[NetworkServer] Unhandled packet type: " << (int)type << std::endl;
            break;
    }
}

void NetworkServer::handleLogin(ENetPeer* peer, PacketReader& reader) {
    std::string username = reader.readString();
    std::string password = reader.readString();

    std::cout << "[NetworkServer] Login request: " << username << std::endl;

    // TODO: Actual authentication with database
    // For now, auto-succeed

    uint32_t clientId = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(peer->data));
    auto it = clients.find(clientId);
    
    if (it != clients.end()) {
        it->second.authenticated = true;
        it->second.playerId = clientId; // Use clientId as playerId for now

        // Send success response
        PacketBuilder builder(PacketType::AUTH_LOGIN_RESPONSE);
        builder.writeUInt8(1); // Success
        builder.writeUInt32(clientId); // Player ID
        builder.writeUInt64(12345); // Session token (dummy)

        sendPacket(peer, builder.getData(), builder.getSize(), ENetChannel::RELIABLE_ORDERED);

        std::cout << "[NetworkServer] Login successful: " << username << " (ID: " << clientId << ")" << std::endl;
    }
}

void NetworkServer::handleRegister(ENetPeer* peer, PacketReader& reader) {
    std::string username = reader.readString();
    std::string password = reader.readString();

    std::cout << "[NetworkServer] Register request: " << username << std::endl;

    // TODO: Actual registration with database
    
    PacketBuilder builder(PacketType::AUTH_REGISTER_RESPONSE);
    builder.writeUInt8(1); // Success
    builder.writeString("Registration successful");

    sendPacket(peer, builder.getData(), builder.getSize(), ENetChannel::RELIABLE_ORDERED);
}

void NetworkServer::handlePlayerMove(ENetPeer* peer, PacketReader& reader) {
    PlayerState state = reader.readPlayerState();

    // TODO: Validate movement (anti-cheat)
    // TODO: Update player manager
    
    // Broadcast to other players
    PacketBuilder builder(PacketType::PLAYER_MOVE);
    builder.writePlayerState(state);
    
    // Send to all except sender
    for (auto& pair : clients) {
        if (pair.second.peer != peer && pair.second.authenticated) {
            sendPacket(pair.second.peer, builder.getData(), builder.getSize(), 
                      ENetChannel::UNRELIABLE_SEQUENCED);
        }
    }
}

void NetworkServer::handleWeaponShoot(ENetPeer* peer, PacketReader& reader) {
    uint32_t shooterId = reader.readUInt32();
    Vector3 origin = reader.readVector3();
    Vector3 direction = reader.readVector3();
    uint16_t weaponId = reader.readUInt16();
    uint32_t timestamp = reader.readUInt32();

    std::cout << "[NetworkServer] Weapon fire from player " << shooterId << std::endl;

    // TODO: Server-side hit detection
    // TODO: Validate shot (anti-cheat)
    
    // Broadcast to all other players
    PacketBuilder builder(PacketType::WEAPON_SHOOT);
    builder.writeUInt32(shooterId);
    builder.writeVector3(origin);
    builder.writeVector3(direction);

    for (auto& pair : clients) {
        if (pair.second.peer != peer && pair.second.authenticated) {
            sendPacket(pair.second.peer, builder.getData(), builder.getSize(),
                      ENetChannel::UNRELIABLE_SEQUENCED);
        }
    }
}

} // namespace TDS
