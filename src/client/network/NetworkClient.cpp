#include "NetworkClient.h"
#include <cstring>

NetworkClient::NetworkClient() : serverSocket(INVALID_SOCKET), initialized(false),
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

NetworkClient::~NetworkClient() {
    disconnect();
    if (initialized) {
        WSACleanup();
    }
}

bool NetworkClient::connect(const std::string& serverIP, int port) {
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

void NetworkClient::disconnect() {
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

void NetworkClient::update() {
    if (!connected) return;
    receiveData();
}

bool NetworkClient::sendPacket(PacketType type, const void* payload, uint32_t payloadSize) {
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

bool NetworkClient::isConnected() const {
    return connected;
}

void NetworkClient::setSessionToken(uint64_t token) {
    sessionToken = token;
}

uint64_t NetworkClient::getSessionToken() const {
    return sessionToken;
}

bool NetworkClient::hasPackets() const {
    return !receivedPackets.empty();
}

NetworkClient::ReceivedPacket NetworkClient::getNextPacket() {
    if (receivedPackets.empty()) {
        return ReceivedPacket();
    }

    ReceivedPacket packet = receivedPackets.front();
    receivedPackets.pop();
    return packet;
}

std::vector<NetworkClient::ReceivedPacket> NetworkClient::getAllPackets() {
    std::vector<ReceivedPacket> packets;
    while (!receivedPackets.empty()) {
        packets.push_back(receivedPackets.front());
        receivedPackets.pop();
    }
    return packets;
}

void NetworkClient::receiveData() {
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

void NetworkClient::parsePackets() {
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
