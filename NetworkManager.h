#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <string>
#include <queue>
#include <memory>
#include <functional>
#include <cstdint>
#include "CivilizationAI.h"

#pragma comment(lib, "ws2_32.lib")

// Network packet types
enum class PacketType : uint8_t {
    // Connection packets
    CONNECT_REQUEST = 0,
    CONNECT_RESPONSE = 1,
    DISCONNECT = 2,
    PING = 3,
    PONG = 4,

    // Game state sync
    WORLD_STATE_FULL = 10,
    WORLD_STATE_DELTA = 11,
    AGENT_UPDATE = 12,
    BUILDING_UPDATE = 13,

    // Player commands
    PLAYER_INPUT = 20,
    SELECT_AGENT = 21,
    SELECT_BUILDING = 22,
    CHANGE_ROLE = 23,
    BUILD_ORDER = 24,

    // Game events
    AGENT_SPAWN = 30,
    AGENT_DEATH = 31,
    BUILDING_CONSTRUCT = 32,
    BUILDING_DESTROY = 33,
    COMBAT_EVENT = 34,
    RESOURCE_COLLECT = 35,

    // Advanced features
    DIPLOMACY_ACTION = 40,
    TRADE_OFFER = 41,
    TECH_RESEARCH = 42,
    FACTION_POLICY = 43,
    ALLIANCE_FORMED = 44,
    WAR_DECLARED = 45,

    CHAT_MESSAGE = 50
};

// Network packet structure
struct NetworkPacket {
    PacketType type;
    uint32_t size;
    std::vector<uint8_t> data;

    NetworkPacket() : type(PacketType::PING), size(0) {}
    NetworkPacket(PacketType t) : type(t), size(0) {}

    // Serialization helpers
    void writeInt32(int32_t value);
    void writeFloat(float value);
    void writeString(const std::string& str);
    void writeVector3(const Vector3& vec);
    void writeBool(bool value);
    void writeUInt8(uint8_t value);
    void writeUInt32(uint32_t value);

    int32_t readInt32(size_t& offset) const;
    float readFloat(size_t& offset) const;
    std::string readString(size_t& offset) const;
    Vector3 readVector3(size_t& offset) const;
    bool readBool(size_t& offset) const;
    uint8_t readUInt8(size_t& offset) const;
    uint32_t readUInt32(size_t& offset) const;

    // Convert to/from network format
    std::vector<uint8_t> serialize() const;
    static NetworkPacket deserialize(const std::vector<uint8_t>& buffer);
};

// Client connection info
struct ClientConnection {
    SOCKET socket;
    sockaddr_in address;
    uint32_t clientId;
    bool authenticated;
    std::string playerName;
    Faction controlledFaction;
    float lastPingTime;
    float ping;
};

// Network statistics
struct NetworkStats {
    uint32_t packetsSent;
    uint32_t packetsReceived;
    uint32_t bytesSent;
    uint32_t bytesReceived;
    float averagePing;
    uint32_t packetsLost;

    NetworkStats() : packetsSent(0), packetsReceived(0), bytesSent(0),
                     bytesReceived(0), averagePing(0), packetsLost(0) {}
};

// Base NetworkManager class
class NetworkManager {
protected:
    SOCKET mainSocket;
    bool initialized;
    NetworkStats stats;
    std::queue<NetworkPacket> incomingPackets;
    std::queue<NetworkPacket> outgoingPackets;

public:
    NetworkManager();
    virtual ~NetworkManager();

    virtual bool initialize() = 0;
    virtual void shutdown();
    virtual void update(float deltaTime) = 0;

    // Packet handling
    void queuePacket(const NetworkPacket& packet);
    bool hasPackets() const { return !incomingPackets.empty(); }
    NetworkPacket getNextPacket();

    // Statistics
    const NetworkStats& getStats() const { return stats; }

protected:
    bool initializeWinsock();
    void cleanupWinsock();
    int sendData(SOCKET sock, const uint8_t* data, int length);
    int receiveData(SOCKET sock, uint8_t* buffer, int maxLength);
};

// Server-side network manager
class ServerNetworkManager : public NetworkManager {
private:
    std::vector<ClientConnection> clients;
    uint32_t nextClientId;
    int port;
    bool listening;

public:
    ServerNetworkManager(int serverPort = 27015);
    ~ServerNetworkManager() override;

    bool initialize() override;
    void update(float deltaTime) override;
    void shutdown() override;

    // Client management
    void acceptNewClients();
    void disconnectClient(uint32_t clientId);
    void broadcastPacket(const NetworkPacket& packet);
    void sendToClient(uint32_t clientId, const NetworkPacket& packet);

    // Game state synchronization
    void broadcastWorldState(const World* world);
    void broadcastAgentUpdate(const Agent* agent);
    void broadcastBuildingUpdate(const Building* building);

    // Event broadcasting
    void broadcastCombatEvent(const Agent* attacker, const Agent* defender, float damage);
    void broadcastAgentSpawn(const Agent* agent);
    void broadcastAgentDeath(const Agent* agent);

    // Getters
    size_t getClientCount() const { return clients.size(); }
    const std::vector<ClientConnection>& getClients() const { return clients; }

private:
    void handleClientData(ClientConnection& client);
    void processClientPacket(ClientConnection& client, const NetworkPacket& packet);
    void removeDisconnectedClients();
};

// Client-side network manager
class ClientNetworkManager : public NetworkManager {
private:
    std::string serverAddress;
    int serverPort;
    bool connected;
    float reconnectTimer;
    uint32_t clientId;
    std::string playerName;
    Faction faction;

    // Latency tracking
    float lastPingTime;
    float currentPing;

public:
    ClientNetworkManager(const std::string& server = "127.0.0.1", int port = 27015);
    ~ClientNetworkManager() override;

    bool initialize() override;
    void update(float deltaTime) override;
    void shutdown() override;

    // Connection management
    bool connectToServer();
    void disconnect();
    bool isConnected() const { return connected; }

    // Send commands to server
    void sendPlayerInput(const Vector3& clickPosition);
    void sendSelectAgent(uint32_t agentId);
    void sendSelectBuilding(uint32_t buildingId);
    void sendChangeRole(uint32_t agentId, Role newRole);
    void sendBuildOrder(Building::Type type, const Vector3& position);
    void sendChatMessage(const std::string& message);

    // Getters
    uint32_t getClientId() const { return clientId; }
    float getPing() const { return currentPing; }
    Faction getFaction() const { return faction; }

private:
    void handleServerData();
    void processServerPacket(const NetworkPacket& packet);
    void sendPing();
};

#endif // NETWORK_MANAGER_H
