#include "NetworkServer.h"
#include <iostream>
#include <cstring>

NetworkServer::NetworkServer() : listenSocket(INVALID_SOCKET), initialized(false), running(false) {
    // Initialize Winsock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cout << "[NetworkServer] WSAStartup failed: " << result << std::endl;
        return;
    }
    initialized = true;
    std::cout << "[NetworkServer] Winsock initialized" << std::endl;
}

NetworkServer::~NetworkServer() {
    shutdown();
    if (initialized) {
        WSACleanup();
    }
}

bool NetworkServer::start(int port) {
    if (!initialized) {
        std::cout << "[NetworkServer] Cannot start - not initialized" << std::endl;
        return false;
    }

    // Create listen socket
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cout << "[NetworkServer] Failed to create socket: " << WSAGetLastError() << std::endl;
        return false;
    }

    // Set socket to non-blocking
    u_long mode = 1;
    ioctlsocket(listenSocket, FIONBIO, &mode);

    // Bind socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cout << "[NetworkServer] Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return false;
    }

    // Listen
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cout << "[NetworkServer] Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return false;
    }

    running = true;
    serverPort = port;
    std::cout << "[NetworkServer] Server started on port " << port << std::endl;
    return true;
}

void NetworkServer::shutdown() {
    if (!running) return;

    running = false;

    // Close all client connections
    for (auto& pair : clients) {
        closesocket(pair.second.socket);
    }
    clients.clear();

    // Close listen socket
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }

    std::cout << "[NetworkServer] Server shutdown" << std::endl;
}

void NetworkServer::update() {
    if (!running) return;

    // Accept new connections
    acceptNewConnections();

    // Receive data from all clients
    receiveFromAllClients();

    // Remove disconnected clients
    removeDisconnectedClients();
}

bool NetworkServer::sendPacket(uint64_t clientId, PacketType type, const void* payload, uint32_t payloadSize, uint64_t sessionToken) {
    auto it = clients.find(clientId);
    if (it == clients.end()) {
        return false;
    }

    // Build packet
    PacketHeader header;
    header.type = static_cast<uint16_t>(type);
    header.payloadSize = payloadSize;
    header.sessionToken = sessionToken;
    header.sequence = it->second.sequenceOut++;

    // Send header
    int result = send(it->second.socket, (char*)&header, sizeof(header), 0);
    if (result == SOCKET_ERROR) {
        std::cout << "[NetworkServer] Failed to send header to client " << clientId << ": " << WSAGetLastError() << std::endl;
        it->second.connected = false;
        return false;
    }

    // Send payload (if any)
    if (payloadSize > 0 && payload != nullptr) {
        result = send(it->second.socket, (char*)payload, payloadSize, 0);
        if (result == SOCKET_ERROR) {
            std::cout << "[NetworkServer] Failed to send payload to client " << clientId << ": " << WSAGetLastError() << std::endl;
            it->second.connected = false;
            return false;
        }
    }

    return true;
}

void NetworkServer::broadcastPacket(PacketType type, const void* payload, uint32_t payloadSize, uint64_t sessionToken) {
    for (auto& pair : clients) {
        sendPacket(pair.first, type, payload, payloadSize, sessionToken);
    }
}

void NetworkServer::broadcastToClients(const std::vector<uint64_t>& clientIds, PacketType type, const void* payload, uint32_t payloadSize, uint64_t sessionToken) {
    for (uint64_t clientId : clientIds) {
        sendPacket(clientId, type, payload, payloadSize, sessionToken);
    }
}

bool NetworkServer::isClientConnected(uint64_t clientId) const {
    auto it = clients.find(clientId);
    return it != clients.end() && it->second.connected;
}

int NetworkServer::getClientCount() const {
    return static_cast<int>(clients.size());
}

void NetworkServer::disconnectClient(uint64_t clientId) {
    auto it = clients.find(clientId);
    if (it != clients.end()) {
        closesocket(it->second.socket);
        clients.erase(it);
        std::cout << "[NetworkServer] Client " << clientId << " disconnected" << std::endl;
    }
}

std::vector<NetworkServer::ReceivedPacket> NetworkServer::getReceivedPackets() {
    std::vector<ReceivedPacket> packets = receivedPackets;
    receivedPackets.clear();
    return packets;
}

void NetworkServer::acceptNewConnections() {
    SOCKET clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            std::cout << "[NetworkServer] Accept failed: " << error << std::endl;
        }
        return;
    }

    // Set client socket to non-blocking
    u_long mode = 1;
    ioctlsocket(clientSocket, FIONBIO, &mode);

    // Create client connection
    ClientConnection client;
    client.socket = clientSocket;
    client.clientId = nextClientId++;

    // Get client IP
    sockaddr_in clientAddr;
    int addrLen = sizeof(clientAddr);
    getpeername(clientSocket, (sockaddr*)&clientAddr, &addrLen);
    char ipStr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &clientAddr.sin_addr, ipStr, INET_ADDRSTRLEN);
    client.ipAddress = ipStr;

    clients[client.clientId] = client;
    std::cout << "[NetworkServer] New client connected: " << client.clientId << " (" << ipStr << ")" << std::endl;
}

void NetworkServer::receiveFromAllClients() {
    for (auto& pair : clients) {
        receiveFromClient(pair.second);
    }
}

void NetworkServer::receiveFromClient(ClientConnection& client) {
    if (!client.connected) return;

    char buffer[4096];
    int result = recv(client.socket, buffer, sizeof(buffer), 0);

    if (result > 0) {
        // Append to receive buffer
        client.receiveBuffer.insert(client.receiveBuffer.end(), buffer, buffer + result);

        // Try to parse packets
        parseClientPackets(client);
    }
    else if (result == 0) {
        // Connection closed
        client.connected = false;
        std::cout << "[NetworkServer] Client " << client.clientId << " closed connection" << std::endl;
    }
    else {
        int error = WSAGetLastError();
        if (error != WSAEWOULDBLOCK) {
            client.connected = false;
            std::cout << "[NetworkServer] Receive failed from client " << client.clientId << ": " << error << std::endl;
        }
    }
}

void NetworkServer::parseClientPackets(ClientConnection& client) {
    while (client.receiveBuffer.size() >= sizeof(PacketHeader)) {
        // Read header
        PacketHeader header;
        memcpy(&header, client.receiveBuffer.data(), sizeof(PacketHeader));

        // Check if we have the full packet
        size_t totalSize = sizeof(PacketHeader) + header.payloadSize;
        if (client.receiveBuffer.size() < totalSize) {
            break;  // Wait for more data
        }

        // Extract payload
        std::vector<uint8_t> payload;
        if (header.payloadSize > 0) {
            payload.resize(header.payloadSize);
            memcpy(payload.data(), client.receiveBuffer.data() + sizeof(PacketHeader), header.payloadSize);
        }

        // Create received packet
        ReceivedPacket packet;
        packet.clientId = client.clientId;
        packet.type = static_cast<PacketType>(header.type);
        packet.payload = payload;
        packet.sessionToken = header.sessionToken;
        receivedPackets.push_back(packet);

        // Remove processed packet from buffer
        client.receiveBuffer.erase(client.receiveBuffer.begin(), client.receiveBuffer.begin() + totalSize);

        std::cout << "[NetworkServer] Received " << packetTypeToString(packet.type)
                  << " from client " << client.clientId << std::endl;
    }
}

void NetworkServer::removeDisconnectedClients() {
    std::vector<uint64_t> toRemove;
    for (auto& pair : clients) {
        if (!pair.second.connected) {
            toRemove.push_back(pair.first);
        }
    }

    for (uint64_t clientId : toRemove) {
        closesocket(clients[clientId].socket);
        clients.erase(clientId);
    }
}
