// NAT Punchthrough Client
// Integrates with game client to enable P2P connections through NAT

#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <chrono>
#include <functional>

#pragma comment(lib, "ws2_32.lib")

// Forward declarations from NatPunchServer.h
enum class NatPacketType : uint8_t {
    REGISTER = 1,
    REGISTER_ACK = 2,
    HEARTBEAT = 3,
    HEARTBEAT_ACK = 4,
    REQUEST_PUNCH = 5,
    PUNCH_INFO = 6,
    UNREGISTER = 7,
    LIST_CLIENTS = 8,
    CLIENT_LIST = 9
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
    uint32_t ipAddress;
    uint16_t port;
};

struct ClientListEntry {
    char clientId[64];
};
#pragma pack(pop)

struct PeerInfo {
    std::string clientId;
    std::string ipAddress;
    uint16_t port;
};

class NatPunchClient {
private:
    SOCKET m_socket;
    std::string m_clientId;
    std::string m_serverAddress;
    uint16_t m_serverPort;
    sockaddr_in m_serverAddr;
    bool m_registered;
    std::chrono::steady_clock::time_point m_lastHeartbeat;

    std::function<void(const PeerInfo&)> m_onPeerInfoReceived;

    static constexpr int MAX_PACKET_SIZE = 4096;
    static constexpr int HEARTBEAT_INTERVAL_SECONDS = 10;

public:
    NatPunchClient(const std::string& clientId, const std::string& serverAddr = "127.0.0.1", uint16_t serverPort = 3478)
        : m_socket(INVALID_SOCKET)
        , m_clientId(clientId)
        , m_serverAddress(serverAddr)
        , m_serverPort(serverPort)
        , m_registered(false)
    {
        memset(&m_serverAddr, 0, sizeof(m_serverAddr));
    }

    ~NatPunchClient() {
        disconnect();
    }

    bool connect() {
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "[NatClient] WSAStartup failed!" << std::endl;
            return false;
        }

        // Create UDP socket
        m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (m_socket == INVALID_SOCKET) {
            std::cout << "[NatClient] Failed to create socket!" << std::endl;
            WSACleanup();
            return false;
        }

        // Set non-blocking mode
        u_long mode = 1;
        ioctlsocket(m_socket, FIONBIO, &mode);

        // Prepare server address
        m_serverAddr.sin_family = AF_INET;
        m_serverAddr.sin_port = htons(m_serverPort);
        inet_pton(AF_INET, m_serverAddress.c_str(), &m_serverAddr.sin_addr);

        std::cout << "[NatClient] Connected to NAT server: " << m_serverAddress << ":" << m_serverPort << std::endl;

        // Register with server
        return registerWithServer();
    }

    void disconnect() {
        if (m_socket != INVALID_SOCKET) {
            if (m_registered) {
                unregisterFromServer();
            }

            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
            WSACleanup();
        }
    }

    void update() {
        if (m_socket == INVALID_SOCKET) return;

        // Send heartbeat if needed
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastHeartbeat).count();
        if (elapsed >= HEARTBEAT_INTERVAL_SECONDS) {
            sendHeartbeat();
            m_lastHeartbeat = now;
        }

        // Process incoming packets
        processIncomingPackets();
    }

    void requestPunch(const std::string& targetClientId) {
        char buffer[sizeof(NatPacketHeader) + sizeof(PunchRequestPacket)];
        NatPacketHeader* header = (NatPacketHeader*)buffer;
        header->type = NatPacketType::REQUEST_PUNCH;
        header->payloadSize = sizeof(PunchRequestPacket);

        PunchRequestPacket* packet = (PunchRequestPacket*)(buffer + sizeof(NatPacketHeader));
        strncpy_s(packet->targetClientId, targetClientId.c_str(), sizeof(packet->targetClientId) - 1);

        sendto(m_socket, buffer, sizeof(buffer), 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));

        std::cout << "[NatClient] Requested punch to: " << targetClientId << std::endl;
    }

    void setOnPeerInfoReceived(std::function<void(const PeerInfo&)> callback) {
        m_onPeerInfoReceived = callback;
    }

    bool isRegistered() const { return m_registered; }

    // Helper function to send UDP packet directly to peer (for hole punching)
    void sendToPeer(const std::string& ipAddress, uint16_t port, const char* data, int size) {
        sockaddr_in peerAddr{};
        peerAddr.sin_family = AF_INET;
        peerAddr.sin_port = htons(port);
        inet_pton(AF_INET, ipAddress.c_str(), &peerAddr.sin_addr);

        sendto(m_socket, data, size, 0, (sockaddr*)&peerAddr, sizeof(peerAddr));
    }

    SOCKET getSocket() const { return m_socket; }

private:
    bool registerWithServer() {
        char buffer[sizeof(NatPacketHeader) + sizeof(RegisterPacket)];
        NatPacketHeader* header = (NatPacketHeader*)buffer;
        header->type = NatPacketType::REGISTER;
        header->payloadSize = sizeof(RegisterPacket);

        RegisterPacket* packet = (RegisterPacket*)(buffer + sizeof(NatPacketHeader));
        strncpy_s(packet->clientId, m_clientId.c_str(), sizeof(packet->clientId) - 1);

        sendto(m_socket, buffer, sizeof(buffer), 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));

        std::cout << "[NatClient] Sent registration as: " << m_clientId << std::endl;

        // Wait for acknowledgment (with timeout)
        auto startTime = std::chrono::steady_clock::now();
        while (true) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();

            if (elapsed > 5000) {
                std::cout << "[NatClient] Registration timeout!" << std::endl;
                return false;
            }

            char recvBuffer[MAX_PACKET_SIZE];
            sockaddr_in from{};
            int fromLen = sizeof(from);

            int bytesReceived = recvfrom(m_socket, recvBuffer, MAX_PACKET_SIZE, 0,
                                        (sockaddr*)&from, &fromLen);

            if (bytesReceived > 0 && bytesReceived >= sizeof(NatPacketHeader)) {
                NatPacketHeader* recvHeader = (NatPacketHeader*)recvBuffer;
                if (recvHeader->type == NatPacketType::REGISTER_ACK) {
                    m_registered = true;
                    m_lastHeartbeat = std::chrono::steady_clock::now();
                    std::cout << "[NatClient] Registration successful!" << std::endl;
                    return true;
                }
            }

            Sleep(10);
        }
    }

    void sendHeartbeat() {
        char buffer[sizeof(NatPacketHeader) + sizeof(RegisterPacket)];
        NatPacketHeader* header = (NatPacketHeader*)buffer;
        header->type = NatPacketType::HEARTBEAT;
        header->payloadSize = sizeof(RegisterPacket);

        RegisterPacket* packet = (RegisterPacket*)(buffer + sizeof(NatPacketHeader));
        strncpy_s(packet->clientId, m_clientId.c_str(), sizeof(packet->clientId) - 1);

        sendto(m_socket, buffer, sizeof(buffer), 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
    }

    void unregisterFromServer() {
        char buffer[sizeof(NatPacketHeader) + sizeof(RegisterPacket)];
        NatPacketHeader* header = (NatPacketHeader*)buffer;
        header->type = NatPacketType::UNREGISTER;
        header->payloadSize = sizeof(RegisterPacket);

        RegisterPacket* packet = (RegisterPacket*)(buffer + sizeof(NatPacketHeader));
        strncpy_s(packet->clientId, m_clientId.c_str(), sizeof(packet->clientId) - 1);

        sendto(m_socket, buffer, sizeof(buffer), 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));

        m_registered = false;
        std::cout << "[NatClient] Unregistered from NAT server" << std::endl;
    }

    void processIncomingPackets() {
        char buffer[MAX_PACKET_SIZE];
        sockaddr_in from{};
        int fromLen = sizeof(from);

        while (true) {
            int bytesReceived = recvfrom(m_socket, buffer, MAX_PACKET_SIZE, 0,
                                        (sockaddr*)&from, &fromLen);

            if (bytesReceived == SOCKET_ERROR) {
                int error = WSAGetLastError();
                if (error != WSAEWOULDBLOCK) {
                    std::cout << "[NatClient] recvfrom error: " << error << std::endl;
                }
                break;
            }

            if (bytesReceived < sizeof(NatPacketHeader)) continue;

            NatPacketHeader* header = (NatPacketHeader*)buffer;
            char* payload = buffer + sizeof(NatPacketHeader);

            if (header->type == NatPacketType::PUNCH_INFO) {
                handlePunchInfo(payload, header->payloadSize);
            }
            else if (header->type == NatPacketType::HEARTBEAT_ACK) {
                // Heartbeat acknowledged, nothing to do
            }
        }
    }

    void handlePunchInfo(char* payload, uint16_t size) {
        if (size < sizeof(PunchInfoPacket)) return;

        PunchInfoPacket* packet = (PunchInfoPacket*)payload;

        PeerInfo peer;
        peer.clientId = std::string(packet->targetClientId);

        // Convert IP address to string
        char addrStr[INET_ADDRSTRLEN];
        struct in_addr addr;
        addr.s_addr = packet->ipAddress;
        inet_ntop(AF_INET, &addr, addrStr, INET_ADDRSTRLEN);
        peer.ipAddress = std::string(addrStr);
        peer.port = ntohs(packet->port);

        std::cout << "[NatClient] Received peer info: " << peer.clientId
                  << " at " << peer.ipAddress << ":" << peer.port << std::endl;

        // Initiate hole punch by sending a packet to the peer
        const char* punchMsg = "PUNCH";
        sendToPeer(peer.ipAddress, peer.port, punchMsg, static_cast<int>(strlen(punchMsg)));

        std::cout << "[NatClient] Sent hole punch packet to peer" << std::endl;

        // Notify callback
        if (m_onPeerInfoReceived) {
            m_onPeerInfoReceived(peer);
        }
    }
};
