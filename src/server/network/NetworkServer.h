#pragma once
#include "../../engine/core/Platform.h"
#include "../../common/NetworkProtocol.h"
#include "../../common/DataStructures.h"
#include <vector>
#include <map>
#include <memory>

// Server-side network manager
class NetworkServer {
public:
    NetworkServer();
    ~NetworkServer();

    // Start listening on specified port
    bool start(int port = 7777);

    // Shutdown server
    void shutdown();

    // Update server (accept new connections, receive data)
    void update();

    // Send packet to specific client
    bool sendPacket(uint64_t clientId, PacketType type, const void* payload, uint32_t payloadSize, uint64_t sessionToken = 0);

    // Broadcast packet to all clients
    void broadcastPacket(PacketType type, const void* payload, uint32_t payloadSize, uint64_t sessionToken = 0);

    // Broadcast packet to specific clients
    void broadcastToClients(const std::vector<uint64_t>& clientIds, PacketType type, const void* payload, uint32_t payloadSize, uint64_t sessionToken = 0);

    // Check if client is connected
    bool isClientConnected(uint64_t clientId) const;

    // Get number of connected clients
    int getClientCount() const;

    // Disconnect client
    void disconnectClient(uint64_t clientId);

    // Packet queue for processing
    struct ReceivedPacket {
        uint64_t clientId;
        PacketType type;
        std::vector<uint8_t> payload;
        uint64_t sessionToken;
    };

    // Get all received packets
    std::vector<ReceivedPacket> getReceivedPackets();

private:
    struct ClientConnection {
        SOCKET socket;
        uint64_t clientId;
        std::string ipAddress;
        bool connected;
        uint32_t sequenceIn;
        uint32_t sequenceOut;
        std::vector<uint8_t> receiveBuffer;

        ClientConnection() : socket(INVALID_SOCKET), clientId(0), connected(true),
                            sequenceIn(0), sequenceOut(0) {}
    };

    SOCKET listenSocket;
    std::map<uint64_t, ClientConnection> clients;
    std::vector<ReceivedPacket> receivedPackets;
    uint64_t nextClientId = 1;
    int serverPort = 0;
    bool initialized;
    bool running;

    void acceptNewConnections();
    void receiveFromAllClients();
    void receiveFromClient(ClientConnection& client);
    void parseClientPackets(ClientConnection& client);
    void removeDisconnectedClients();
};
