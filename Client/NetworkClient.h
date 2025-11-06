#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <vector>
#include <queue>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

// Client-side network manager
class NetworkClient {
public:
    NetworkClient();
    ~NetworkClient();

    // Connect to server
    bool connect(const std::string& serverIP, int port = 7777);

    // Disconnect from server
    void disconnect();

    // Update (receive data)
    void update();

    // Send packet to server
    bool sendPacket(PacketType type, const void* payload, uint32_t payloadSize);

    // Check if connected
    bool isConnected() const;

    // Set session token (after login)
    void setSessionToken(uint64_t token);

    // Get session token
    uint64_t getSessionToken() const;

    // Received packet structure
    struct ReceivedPacket {
        PacketType type;
        std::vector<uint8_t> payload;
    };

    // Check if packets available
    bool hasPackets() const;

    // Get next received packet
    ReceivedPacket getNextPacket();

    // Get all received packets
    std::vector<ReceivedPacket> getAllPackets();

private:
    SOCKET serverSocket;
    bool initialized;
    bool connected;
    uint64_t sessionToken;
    uint32_t sequenceOut;
    std::vector<uint8_t> receiveBuffer;
    std::queue<ReceivedPacket> receivedPackets;

    void receiveData();
    void parsePackets();
};
