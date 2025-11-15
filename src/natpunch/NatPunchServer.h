// NAT Punchthrough Server
// Facilitates UDP hole punching for peer-to-peer connections

#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>

#pragma comment(lib, "ws2_32.lib")

// Client info stored by NAT server
struct NatClient {
    std::string clientId;
    sockaddr_in address;
    std::chrono::steady_clock::time_point lastHeartbeat;
    bool isActive;

    NatClient() : isActive(false) {}

    NatClient(const std::string& id, const sockaddr_in& addr)
        : clientId(id), address(addr), lastHeartbeat(std::chrono::steady_clock::now()), isActive(true) {}
};

// NAT Punchthrough Protocol
enum class NatPacketType : uint8_t {
    REGISTER = 1,           // Client registers with NAT server
    REGISTER_ACK = 2,       // Server acknowledges registration
    HEARTBEAT = 3,          // Client sends heartbeat
    HEARTBEAT_ACK = 4,      // Server responds to heartbeat
    REQUEST_PUNCH = 5,      // Request connection to another client
    PUNCH_INFO = 6,         // Server sends peer's address info
    UNREGISTER = 7,         // Client unregisters
    LIST_CLIENTS = 8,       // Request list of active clients
    CLIENT_LIST = 9         // Response with client list
};

#pragma pack(push, 1)
struct NatPacketHeader {
    NatPacketType type;
    uint16_t payloadSize;
};

struct RegisterPacket {
    char clientId[64];
};

struct PunchRequestPacket {
    char targetClientId[64];
};

struct PunchInfoPacket {
    char targetClientId[64];
    uint32_t ipAddress;     // Network byte order
    uint16_t port;          // Network byte order
};

struct ClientListEntry {
    char clientId[64];
};
#pragma pack(pop)

class NatPunchServer {
private:
    SOCKET m_socket;
    bool m_running;
    uint16_t m_port;
    std::unordered_map<std::string, NatClient> m_clients;

    static constexpr int HEARTBEAT_TIMEOUT_SECONDS = 30;
    static constexpr int MAX_PACKET_SIZE = 4096;

public:
    NatPunchServer(uint16_t port = 3478) : m_socket(INVALID_SOCKET), m_running(false), m_port(port) {}

    ~NatPunchServer() {
        stop();
    }

    bool start() {
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "[NatServer] WSAStartup failed!" << std::endl;
            return false;
        }

        // Create UDP socket
        m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_socket == INVALID_SOCKET) {
            std::cout << "[NatServer] Failed to create socket!" << std::endl;
            WSACleanup();
            return false;
        }

        // Set non-blocking mode
        u_long mode = 1;
        ioctlsocket(m_socket, FIONBIO, &mode);

        // Bind to port
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(m_port);

        if (bind(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cout << "[NatServer] Failed to bind to port " << m_port << std::endl;
            closesocket(m_socket);
            WSACleanup();
            return false;
        }

        m_running = true;
        std::cout << "[NatServer] NAT Punchthrough Server started on UDP port " << m_port << std::endl;
        std::cout << "[NatServer] Ready to facilitate P2P connections" << std::endl;

        return true;
    }

    void stop() {
        if (!m_running) return;

        m_running = false;

        if (m_socket != INVALID_SOCKET) {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }

        WSACleanup();
        std::cout << "[NatServer] Server stopped" << std::endl;
    }

    void update() {
        if (!m_running) return;

        // Remove inactive clients
        cleanupInactiveClients();

        // Process incoming packets
        char buffer[MAX_PACKET_SIZE];
        sockaddr_in clientAddr{};
        int clientAddrLen = sizeof(clientAddr);

        while (true) {
            int bytesReceived = recvfrom(m_socket, buffer, MAX_PACKET_SIZE, 0,
                                        (sockaddr*)&clientAddr, &clientAddrLen);

            if (bytesReceived == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK) {
                    std::cout << "[NatServer] recvfrom error: " << error << std::endl;
                }
                break;
            }

            if (bytesReceived < sizeof(NatPacketHeader)) continue;

            handlePacket(buffer, bytesReceived, clientAddr);
        }
    }

    bool isRunning() const { return m_running; }

    int getClientCount() const { return static_cast<int>(m_clients.size()); }

private:
    void handlePacket(char* data, int size, const sockaddr_in& from) {
        NatPacketHeader* header = (NatPacketHeader*)data;
        char* payload = data + sizeof(NatPacketHeader);

        switch (header->type) {
            case NatPacketType::REGISTER:
                handleRegister(payload, header->payloadSize, from);
                break;

            case NatPacketType::HEARTBEAT:
                handleHeartbeat(payload, header->payloadSize, from);
                break;

            case NatPacketType::REQUEST_PUNCH:
                handlePunchRequest(payload, header->payloadSize, from);
                break;

            case NatPacketType::LIST_CLIENTS:
                handleListClients(from);
                break;

            case NatPacketType::UNREGISTER:
                handleUnregister(payload, header->payloadSize, from);
                break;

            default:
                std::cout << "[NatServer] Unknown packet type: " << (int)header->type << std::endl;
                break;
        }
    }

    void handleRegister(char* payload, uint16_t size, const sockaddr_in& from) {
        if (size < sizeof(RegisterPacket)) return;

        RegisterPacket* packet = (RegisterPacket*)payload;
        std::string clientId(packet->clientId);

        // Register or update client
        m_clients[clientId] = NatClient(clientId, from);

        char addrStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &from.sin_addr, addrStr, INET_ADDRSTRLEN);

        std::cout << "[NatServer] Registered client: " << clientId
                  << " from " << addrStr << ":" << ntohs(from.sin_port) << std::endl;

        // Send acknowledgment
        char response[sizeof(NatPacketHeader)];
        NatPacketHeader* header = (NatPacketHeader*)response;
        header->type = NatPacketType::REGISTER_ACK;
        header->payloadSize = 0;

        sendto(m_socket, response, sizeof(NatPacketHeader), 0,
               (sockaddr*)&from, sizeof(from));
    }

    void handleHeartbeat(char* payload, uint16_t size, const sockaddr_in& from) {
        if (size < sizeof(RegisterPacket)) return;

        RegisterPacket* packet = (RegisterPacket*)payload;
        std::string clientId(packet->clientId);

        auto it = m_clients.find(clientId);
        if (it != m_clients.end()) {
            it->second.lastHeartbeat = std::chrono::steady_clock::now();

            // Send acknowledgment
            char response[sizeof(NatPacketHeader)];
            NatPacketHeader* header = (NatPacketHeader*)response;
            header->type = NatPacketType::HEARTBEAT_ACK;
            header->payloadSize = 0;

            sendto(m_socket, response, sizeof(NatPacketHeader), 0,
                   (sockaddr*)&from, sizeof(from));
        }
    }

    void handlePunchRequest(char* payload, uint16_t size, const sockaddr_in& from) {
        if (size < sizeof(PunchRequestPacket)) return;

        PunchRequestPacket* packet = (PunchRequestPacket*)payload;
        std::string targetId(packet->targetClientId);

        auto it = m_clients.find(targetId);
        if (it == m_clients.end()) {
            std::cout << "[NatServer] Punch request failed: target client not found: " << targetId << std::endl;
            return;
        }

        NatClient& target = it->second;

        char addrStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &target.address.sin_addr, addrStr, INET_ADDRSTRLEN);

        std::cout << "[NatServer] Facilitating punch: target " << targetId
                  << " at " << addrStr << ":" << ntohs(target.address.sin_port) << std::endl;

        // Send target's address info back to requester
        char response[sizeof(NatPacketHeader) + sizeof(PunchInfoPacket)];
        NatPacketHeader* header = (NatPacketHeader*)response;
        header->type = NatPacketType::PUNCH_INFO;
        header->payloadSize = sizeof(PunchInfoPacket);

        PunchInfoPacket* info = (PunchInfoPacket*)(response + sizeof(NatPacketHeader));
        strncpy_s(info->targetClientId, targetId.c_str(), sizeof(info->targetClientId) - 1);
        info->ipAddress = target.address.sin_addr.s_addr;
        info->port = target.address.sin_port;

        sendto(m_socket, response, sizeof(response), 0,
               (sockaddr*)&from, sizeof(from));

        // Also send requester's info to target for simultaneous punch
        inet_ntop(AF_INET, &from.sin_addr, addrStr, INET_ADDRSTRLEN);
        std::cout << "[NatServer] Sending reverse punch info to target" << std::endl;

        info->ipAddress = from.sin_addr.s_addr;
        info->port = from.sin_port;
        strncpy_s(info->targetClientId, "requester", sizeof(info->targetClientId) - 1);

        sendto(m_socket, response, sizeof(response), 0,
               (sockaddr*)&target.address, sizeof(target.address));
    }

    void handleListClients(const sockaddr_in& from) {
        std::vector<ClientListEntry> entries;

        for (const auto& pair : m_clients) {
            if (pair.second.isActive) {
                ClientListEntry entry;
                strncpy_s(entry.clientId, pair.first.c_str(), sizeof(entry.clientId) - 1);
                entries.push_back(entry);
            }
        }

        uint16_t payloadSize = static_cast<uint16_t>(entries.size() * sizeof(ClientListEntry));
        char response[sizeof(NatPacketHeader) + 256 * sizeof(ClientListEntry)];

        NatPacketHeader* header = (NatPacketHeader*)response;
        header->type = NatPacketType::CLIENT_LIST;
        header->payloadSize = payloadSize;

        memcpy(response + sizeof(NatPacketHeader), entries.data(), payloadSize);

        sendto(m_socket, response, sizeof(NatPacketHeader) + payloadSize, 0,
               (sockaddr*)&from, sizeof(from));

        std::cout << "[NatServer] Sent client list: " << entries.size() << " clients" << std::endl;
    }

    void handleUnregister(char* payload, uint16_t size, const sockaddr_in& from) {
        if (size < sizeof(RegisterPacket)) return;

        RegisterPacket* packet = (RegisterPacket*)payload;
        std::string clientId(packet->clientId);

        m_clients.erase(clientId);

        std::cout << "[NatServer] Unregistered client: " << clientId << std::endl;
    }

    void cleanupInactiveClients() {
        auto now = std::chrono::steady_clock::now();

        for (auto it = m_clients.begin(); it != m_clients.end(); ) {
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                now - it->second.lastHeartbeat).count();

            if (elapsed > HEARTBEAT_TIMEOUT_SECONDS) {
                std::cout << "[NatServer] Client timed out: " << it->first << std::endl;
                it = m_clients.erase(it);
            } else {
                ++it;
            }
        }
    }
};
