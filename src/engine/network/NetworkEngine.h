#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <queue>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

// Network protocol types
enum class NetProtocol {
    TCP,
    UDP,
    RELIABLE_UDP  // UDP with reliability layer
};

// NAT type detection
enum class NATType {
    UNKNOWN,
    OPEN,                    // No NAT
    FULL_CONE,              // Full cone NAT
    RESTRICTED_CONE,        // Restricted cone NAT
    PORT_RESTRICTED_CONE,   // Port restricted cone NAT
    SYMMETRIC                // Symmetric NAT (hardest to punch)
};

// Connection state
enum class ConnectionState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING,
    FAILED
};

// Packet header for reliable UDP
struct PacketHeader {
    uint32_t sequenceNumber;
    uint32_t ackNumber;
    uint32_t ackBits;
    uint16_t size;
    uint8_t protocol;
    uint8_t flags;
};

// Network packet
struct NetPacket {
    std::vector<uint8_t> data;
    sockaddr_in address;
    bool reliable;
    uint32_t sequenceNumber;
    float timestamp;
    int retransmitCount;

    NetPacket() : reliable(false), sequenceNumber(0), timestamp(0.0f), retransmitCount(0) {}
};

// Peer information
struct Peer {
    std::string id;
    sockaddr_in address;
    sockaddr_in publicAddress;
    sockaddr_in localAddress;
    NATType natType;
    ConnectionState state;
    float lastReceiveTime;
    float lastSendTime;
    float ping;

    // Reliability
    uint32_t localSequence;
    uint32_t remoteSequence;
    std::vector<uint32_t> ackHistory;

    Peer() : natType(NATType::UNKNOWN), state(ConnectionState::DISCONNECTED),
             lastReceiveTime(0.0f), lastSendTime(0.0f), ping(0.0f),
             localSequence(0), remoteSequence(0) {}
};

// STUN server
struct STUNServer {
    std::string hostname;
    uint16_t port;

    STUNServer(const std::string& host = "stun.l.google.com", uint16_t p = 19302)
        : hostname(host), port(p) {}
};

// STUN message
struct STUNMessage {
    uint16_t messageType;
    uint16_t messageLength;
    uint32_t magicCookie;
    uint8_t transactionId[12];
    std::vector<uint8_t> attributes;

    STUNMessage() : messageType(0), messageLength(0), magicCookie(0x2112A442) {
        memset(transactionId, 0, 12);
    }
};

// Network statistics
struct NetworkStats {
    uint64_t bytesSent;
    uint64_t bytesReceived;
    uint64_t packetsSent;
    uint64_t packetsReceived;
    uint64_t packetsLost;
    uint64_t packetsOutOfOrder;
    float averageLatency;
    float packetLoss;
    float bandwidth;  // KB/s

    NetworkStats() : bytesSent(0), bytesReceived(0), packetsSent(0),
                    packetsReceived(0), packetsLost(0), packetsOutOfOrder(0),
                    averageLatency(0.0f), packetLoss(0.0f), bandwidth(0.0f) {}
};

// Network callbacks
class INetworkCallback {
public:
    virtual ~INetworkCallback() = default;
    virtual void onConnected(const std::string& peerId) {}
    virtual void onDisconnected(const std::string& peerId) {}
    virtual void onDataReceived(const std::string& peerId, const uint8_t* data, size_t size) {}
    virtual void onConnectionFailed(const std::string& error) {}
    virtual void onNATTypeDetected(NATType type) {}
};

// Network engine with NAT traversal
class NetworkEngine {
public:
    NetworkEngine();
    ~NetworkEngine();

    // Initialization
    bool initialize(NetProtocol protocol = NetProtocol::RELIABLE_UDP);
    void shutdown();

    // Server
    bool startServer(uint16_t port);
    void stopServer();
    bool isServer() const { return serverMode; }

    // Client
    bool connect(const std::string& address, uint16_t port);
    bool connectViaNAT(const std::string& peerId, const std::string& serverAddress);
    void disconnect();

    // NAT Traversal
    bool detectNATType(const STUNServer& stun = STUNServer());
    NATType getNATType() const { return natType; }
    sockaddr_in getPublicAddress() const { return publicAddress; }
    sockaddr_in getLocalAddress() const { return localAddress; }

    // UDP Hole Punching
    bool punchHole(const sockaddr_in& targetPublic, const sockaddr_in& targetLocal);
    bool sendKeepAlive(const std::string& peerId);

    // Data transmission
    bool send(const std::string& peerId, const void* data, size_t size, bool reliable = true);
    bool sendToAll(const void* data, size_t size, bool reliable = true);
    bool broadcast(const void* data, size_t size);  // Local network broadcast

    // Update
    void update(float deltaTime);
    void pollMessages();

    // Peer management
    Peer* getPeer(const std::string& peerId);
    std::vector<Peer*> getAllPeers();
    int getPeerCount() const { return (int)peers.size(); }

    // Callbacks
    void setCallback(INetworkCallback* callback) { this->callback = callback; }

    // Settings
    void setTimeoutDuration(float seconds) { timeoutDuration = seconds; }
    void setKeepAliveInterval(float seconds) { keepAliveInterval = seconds; }
    void setMaxRetransmits(int max) { maxRetransmits = max; }
    void setPacketLossSimulation(float percentage) { simulatedPacketLoss = percentage; }
    void setLatencySimulation(float ms) { simulatedLatency = ms; }

    // Statistics
    const NetworkStats& getStats() const { return stats; }
    void resetStats();

    // Debug
    void setDebugLogging(bool enabled) { debugLogging = enabled; }

private:
    // Socket operations
    bool createSocket();
    void closeSocket();
    bool bindSocket(uint16_t port);
    bool setNonBlocking();
    bool setReuseAddress();

    // STUN protocol
    bool sendSTUNRequest(const STUNServer& server, STUNMessage& response);
    bool parseSTUNResponse(const std::vector<uint8_t>& data, STUNMessage& message);
    void extractMappedAddress(const STUNMessage& message, sockaddr_in& address);

    // Packet handling
    void processIncomingPackets();
    void processOutgoingPackets();
    void handlePacket(const NetPacket& packet, const sockaddr_in& from);
    void sendPacket(Peer* peer, const void* data, size_t size, bool reliable);

    // Reliability layer
    void sendReliablePacket(Peer* peer, const void* data, size_t size);
    void handleReliablePacket(Peer* peer, const PacketHeader& header, const uint8_t* data, size_t size);
    void updateReliability(Peer* peer, float deltaTime);
    void retransmitPackets(Peer* peer);

    // Connection management
    void checkTimeouts(float deltaTime);
    void updatePing(Peer* peer, float deltaTime);

    // Helpers
    std::string addressToString(const sockaddr_in& addr);
    bool stringToAddress(const std::string& str, sockaddr_in& addr);
    uint16_t getPort(const sockaddr_in& addr);

    // Socket
    SOCKET socket;
    NetProtocol protocol;
    bool serverMode;

    // Addresses
    sockaddr_in localAddress;
    sockaddr_in publicAddress;
    NATType natType;

    // Peers
    std::map<std::string, std::unique_ptr<Peer>> peers;
    std::mutex peersMutex;

    // Packet queues
    std::queue<NetPacket> sendQueue;
    std::queue<NetPacket> receiveQueue;
    std::vector<NetPacket> pendingRetransmits;
    std::mutex queueMutex;

    // Timing
    float timeoutDuration;
    float keepAliveInterval;
    float lastKeepAlive;
    int maxRetransmits;

    // Statistics
    NetworkStats stats;

    // Simulation
    float simulatedPacketLoss;
    float simulatedLatency;

    // Callback
    INetworkCallback* callback;

    // Debug
    bool debugLogging;
};

// Network utility functions
namespace NetworkUtils {
    // Endianness
    uint16_t htons_custom(uint16_t value);
    uint32_t htonl_custom(uint32_t value);
    uint16_t ntohs_custom(uint16_t value);
    uint32_t ntohl_custom(uint32_t value);

    // Checksum
    uint16_t calculateChecksum(const void* data, size_t size);

    // Compression
    std::vector<uint8_t> compress(const void* data, size_t size);
    std::vector<uint8_t> decompress(const void* data, size_t size);

    // Encryption (simple XOR for demo - use proper crypto in production)
    void encrypt(uint8_t* data, size_t size, const std::string& key);
    void decrypt(uint8_t* data, size_t size, const std::string& key);

    // IPv4/IPv6
    bool isIPv4(const std::string& address);
    bool isIPv6(const std::string& address);
    bool isLocalAddress(const sockaddr_in& addr);
}

// Simple signaling server for NAT traversal coordination
class SignalingServer {
public:
    SignalingServer();
    ~SignalingServer();

    bool start(uint16_t port);
    void stop();
    void update();

    // Register peer
    void registerPeer(const std::string& peerId, const sockaddr_in& publicAddr, const sockaddr_in& localAddr);

    // Facilitate connection
    bool facilitateConnection(const std::string& peer1, const std::string& peer2);

private:
    struct RegisteredPeer {
        std::string id;
        sockaddr_in publicAddress;
        sockaddr_in localAddress;
        float lastSeen;
    };

    SOCKET socket;
    std::map<std::string, RegisteredPeer> registeredPeers;
    std::mutex peersMutex;
};
