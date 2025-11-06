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
    NetworkClient() : serverSocket(INVALID_SOCKET), initialized(false),
                      connected(false), sessionToken(0), sequenceOut(0) {
        // Initialize Winsock
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            std::cout << "[NetworkClient] WSAStartup failed: " << result << std::endl;
            return;
        }
        initialized = true;
        std::cout << "[NetworkClient] Winsock initialized" << std::endl;
    }

    ~NetworkClient() {
        disconnect();
        if (initialized) {
            WSACleanup();
        }
    }

    // Connect to server
    bool connect(const std::string& serverIP, int port = 7777) {
        if (!initialized) {
            std::cout << "[NetworkClient] Cannot connect - not initialized" << std::endl;
            return false;
        }

        // Create socket
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            std::cout << "[NetworkClient] Failed to create socket: " << WSAGetLastError() << std::endl;
            return false;
        }

        // Set socket to non-blocking
        u_long mode = 1;
        ioctlsocket(serverSocket, FIONBIO, &mode);

        // Server address
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

        // Connect (non-blocking, will return WOULDBLOCK)
        int result = ::connect(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
        if (result == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                std::cout << "[NetworkClient] Connect failed: " << error << std::endl;
                closesocket(serverSocket);
                serverSocket = INVALID_SOCKET;
                return false;
            }
        }

        // Wait for connection (timeout after 5 seconds)
        fd_set writeSet;
        FD_ZERO(&writeSet);
        FD_SET(serverSocket, &writeSet);

        timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        result = select(0, nullptr, &writeSet, nullptr, &timeout);
        if (result <= 0) {
            std::cout << "[NetworkClient] Connection timeout" << std::endl;
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
            return false;
        }

        connected = true;
        std::cout << "[NetworkClient] Connected to " << serverIP << ":" << port << std::endl;

        return true;
    }

    // Disconnect from server
    void disconnect() {
        if (!connected) return;

        // Send disconnect packet
        sendPacket(PacketType::DISCONNECT, nullptr, 0);

        connected = false;

        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }

        std::cout << "[NetworkClient] Disconnected from server" << std::endl;
    }

    // Update (receive data)
    void update() {
        if (!connected) return;

        receiveData();
    }

    // Send packet to server
    bool sendPacket(PacketType type, const void* payload, uint32_t payloadSize) {
        if (!connected) {
            std::cout << "[NetworkClient] Cannot send - not connected" << std::endl;
            return false;
        }

        // Build packet
        PacketHeader header;
        header.type = static_cast<uint16_t>(type);
        header.payloadSize = payloadSize;
        header.sessionToken = sessionToken;
        header.sequence = sequenceOut++;

        // Send header
        int result = send(serverSocket, (char*)&header, sizeof(header), 0);
        if (result == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                std::cout << "[NetworkClient] Failed to send header: " << error << std::endl;
                connected = false;
                return false;
            }
        }

        // Send payload (if any)
        if (payloadSize > 0 && payload != nullptr) {
            result = send(serverSocket, (char*)payload, payloadSize, 0);
            if (result == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK) {
                    std::cout << "[NetworkClient] Failed to send payload: " << error << std::endl;
                    connected = false;
                    return false;
                }
            }
        }

        return true;
    }

    // Check if connected
    bool isConnected() const {
        return connected;
    }

    // Set session token (after login)
    void setSessionToken(uint64_t token) {
        sessionToken = token;
    }

    // Get session token
    uint64_t getSessionToken() const {
        return sessionToken;
    }

    // Received packet structure
    struct ReceivedPacket {
        PacketType type;
        std::vector<uint8_t> payload;
    };

    // Check if packets available
    bool hasPackets() const {
        return !receivedPackets.empty();
    }

    // Get next received packet
    ReceivedPacket getNextPacket() {
        if (receivedPackets.empty()) {
            return ReceivedPacket();
        }

        ReceivedPacket packet = receivedPackets.front();
        receivedPackets.pop();
        return packet;
    }

    // Get all received packets
    std::vector<ReceivedPacket> getAllPackets() {
        std::vector<ReceivedPacket> packets;
        while (!receivedPackets.empty()) {
            packets.push_back(receivedPackets.front());
            receivedPackets.pop();
        }
        return packets;
    }

private:
    SOCKET serverSocket;
    bool initialized;
    bool connected;
    uint64_t sessionToken;
    uint32_t sequenceOut;
    std::vector<uint8_t> receiveBuffer;
    std::queue<ReceivedPacket> receivedPackets;

    void receiveData() {
        char buffer[4096];
        int result = recv(serverSocket, buffer, sizeof(buffer), 0);

        if (result > 0) {
            // Append to receive buffer
            receiveBuffer.insert(receiveBuffer.end(), buffer, buffer + result);

            // Try to parse packets
            parsePackets();
        }
        else if (result == 0) {
            // Connection closed by server
            connected = false;
            std::cout << "[NetworkClient] Server closed connection" << std::endl;
        }
        else {
            int error = WSAGetLastError();
            if (error != WSAEWOULDBLOCK) {
                connected = false;
                std::cout << "[NetworkClient] Receive failed: " << error << std::endl;
            }
        }
    }

    void parsePackets() {
        while (receiveBuffer.size() >= sizeof(PacketHeader)) {
            // Read header
            PacketHeader header;
            memcpy(&header, receiveBuffer.data(), sizeof(PacketHeader));

            // Check if we have the full packet
            size_t totalSize = sizeof(PacketHeader) + header.payloadSize;
            if (receiveBuffer.size() < totalSize) {
                break;  // Wait for more data
            }

            // Extract payload
            ReceivedPacket packet;
            packet.type = static_cast<PacketType>(header.type);

            if (header.payloadSize > 0) {
                packet.payload.resize(header.payloadSize);
                memcpy(packet.payload.data(), receiveBuffer.data() + sizeof(PacketHeader), header.payloadSize);
            }

            receivedPackets.push(packet);

            // Remove processed packet from buffer
            receiveBuffer.erase(receiveBuffer.begin(), receiveBuffer.begin() + totalSize);

            std::cout << "[NetworkClient] Received " << packetTypeToString(packet.type) << std::endl;
        }
    }
};
