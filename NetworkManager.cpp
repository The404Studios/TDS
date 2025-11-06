#include "NetworkManager.h"
#include <iostream>
#include <cstring>

// NetworkPacket implementation

void NetworkPacket::writeInt32(int32_t value) {
    data.push_back((value >> 0) & 0xFF);
    data.push_back((value >> 8) & 0xFF);
    data.push_back((value >> 16) & 0xFF);
    data.push_back((value >> 24) & 0xFF);
    size += 4;
}

void NetworkPacket::writeFloat(float value) {
    uint32_t intVal;
    memcpy(&intVal, &value, sizeof(float));
    writeUInt32(intVal);
}

void NetworkPacket::writeString(const std::string& str) {
    writeUInt32(static_cast<uint32_t>(str.length()));
    for (char c : str) {
        data.push_back(static_cast<uint8_t>(c));
    }
    size += static_cast<uint32_t>(str.length());
}

void NetworkPacket::writeVector3(const Vector3& vec) {
    writeFloat(vec.x);
    writeFloat(vec.y);
    writeFloat(vec.z);
}

void NetworkPacket::writeBool(bool value) {
    data.push_back(value ? 1 : 0);
    size += 1;
}

void NetworkPacket::writeUInt8(uint8_t value) {
    data.push_back(value);
    size += 1;
}

void NetworkPacket::writeUInt32(uint32_t value) {
    data.push_back((value >> 0) & 0xFF);
    data.push_back((value >> 8) & 0xFF);
    data.push_back((value >> 16) & 0xFF);
    data.push_back((value >> 24) & 0xFF);
    size += 4;
}

int32_t NetworkPacket::readInt32(size_t& offset) const {
    if (offset + 4 > data.size()) return 0;
    int32_t value = data[offset] | (data[offset + 1] << 8) |
                    (data[offset + 2] << 16) | (data[offset + 3] << 24);
    offset += 4;
    return value;
}

float NetworkPacket::readFloat(size_t& offset) const {
    uint32_t intVal = readUInt32(offset);
    float value;
    memcpy(&value, &intVal, sizeof(float));
    return value;
}

std::string NetworkPacket::readString(size_t& offset) const {
    uint32_t length = readUInt32(offset);
    if (offset + length > data.size()) return "";
    std::string str(reinterpret_cast<const char*>(&data[offset]), length);
    offset += length;
    return str;
}

Vector3 NetworkPacket::readVector3(size_t& offset) const {
    Vector3 vec;
    vec.x = readFloat(offset);
    vec.y = readFloat(offset);
    vec.z = readFloat(offset);
    return vec;
}

bool NetworkPacket::readBool(size_t& offset) const {
    if (offset >= data.size()) return false;
    bool value = data[offset] != 0;
    offset += 1;
    return value;
}

uint8_t NetworkPacket::readUInt8(size_t& offset) const {
    if (offset >= data.size()) return 0;
    uint8_t value = data[offset];
    offset += 1;
    return value;
}

uint32_t NetworkPacket::readUInt32(size_t& offset) const {
    if (offset + 4 > data.size()) return 0;
    uint32_t value = data[offset] | (data[offset + 1] << 8) |
                     (data[offset + 2] << 16) | (data[offset + 3] << 24);
    offset += 4;
    return value;
}

std::vector<uint8_t> NetworkPacket::serialize() const {
    std::vector<uint8_t> buffer;
    buffer.push_back(static_cast<uint8_t>(type));

    buffer.push_back((size >> 0) & 0xFF);
    buffer.push_back((size >> 8) & 0xFF);
    buffer.push_back((size >> 16) & 0xFF);
    buffer.push_back((size >> 24) & 0xFF);

    buffer.insert(buffer.end(), data.begin(), data.end());
    return buffer;
}

NetworkPacket NetworkPacket::deserialize(const std::vector<uint8_t>& buffer) {
    NetworkPacket packet;
    if (buffer.size() < 5) return packet;

    packet.type = static_cast<PacketType>(buffer[0]);
    packet.size = buffer[1] | (buffer[2] << 8) | (buffer[3] << 16) | (buffer[4] << 24);

    if (buffer.size() >= 5 + packet.size) {
        packet.data.assign(buffer.begin() + 5, buffer.begin() + 5 + packet.size);
    }

    return packet;
}

// NetworkManager base class

NetworkManager::NetworkManager()
    : mainSocket(INVALID_SOCKET), initialized(false) {
}

NetworkManager::~NetworkManager() {
    shutdown();
}

bool NetworkManager::initializeWinsock() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
    return true;
}

void NetworkManager::cleanupWinsock() {
    WSACleanup();
}

void NetworkManager::shutdown() {
    if (mainSocket != INVALID_SOCKET) {
        closesocket(mainSocket);
        mainSocket = INVALID_SOCKET;
    }
    cleanupWinsock();
    initialized = false;
}

int NetworkManager::sendData(SOCKET sock, const uint8_t* data, int length) {
    int totalSent = 0;
    while (totalSent < length) {
        int sent = send(sock, reinterpret_cast<const char*>(data + totalSent),
                       length - totalSent, 0);
        if (sent == SOCKET_ERROR) {
            return SOCKET_ERROR;
        }
        totalSent += sent;
    }
    stats.packetsSent++;
    stats.bytesSent += totalSent;
    return totalSent;
}

int NetworkManager::receiveData(SOCKET sock, uint8_t* buffer, int maxLength) {
    int received = recv(sock, reinterpret_cast<char*>(buffer), maxLength, 0);
    if (received > 0) {
        stats.packetsReceived++;
        stats.bytesReceived += received;
    }
    return received;
}

void NetworkManager::queuePacket(const NetworkPacket& packet) {
    outgoingPackets.push(packet);
}

NetworkPacket NetworkManager::getNextPacket() {
    if (incomingPackets.empty()) {
        return NetworkPacket();
    }
    NetworkPacket packet = incomingPackets.front();
    incomingPackets.pop();
    return packet;
}

// ServerNetworkManager implementation

ServerNetworkManager::ServerNetworkManager(int serverPort)
    : nextClientId(1), port(serverPort), listening(false) {
}

ServerNetworkManager::~ServerNetworkManager() {
    shutdown();
}

bool ServerNetworkManager::initialize() {
    if (!initializeWinsock()) {
        return false;
    }

    mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mainSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        cleanupWinsock();
        return false;
    }

    // Set socket to non-blocking mode
    u_long mode = 1;
    ioctlsocket(mainSocket, FIONBIO, &mode);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(mainSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(mainSocket);
        cleanupWinsock();
        return false;
    }

    if (listen(mainSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(mainSocket);
        cleanupWinsock();
        return false;
    }

    listening = true;
    initialized = true;
    std::cout << "Server initialized on port " << port << std::endl;
    return true;
}

void ServerNetworkManager::update(float deltaTime) {
    if (!initialized) return;

    // Accept new clients
    acceptNewClients();

    // Handle data from existing clients
    for (auto& client : clients) {
        handleClientData(client);
    }

    // Remove disconnected clients
    removeDisconnectedClients();

    // Process outgoing packets
    while (!outgoingPackets.empty()) {
        NetworkPacket packet = outgoingPackets.front();
        outgoingPackets.pop();
        broadcastPacket(packet);
    }
}

void ServerNetworkManager::acceptNewClients() {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(mainSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket != INVALID_SOCKET) {
        // Set client socket to non-blocking
        u_long mode = 1;
        ioctlsocket(clientSocket, FIONBIO, &mode);

        ClientConnection newClient;
        newClient.socket = clientSocket;
        newClient.address = clientAddr;
        newClient.clientId = nextClientId++;
        newClient.authenticated = false;
        newClient.controlledFaction = Faction::RED;
        newClient.lastPingTime = 0;
        newClient.ping = 0;

        clients.push_back(newClient);

        std::cout << "New client connected: " << newClient.clientId << std::endl;

        // Send connection response
        NetworkPacket response(PacketType::CONNECT_RESPONSE);
        response.writeUInt32(newClient.clientId);
        sendToClient(newClient.clientId, response);
    }
}

void ServerNetworkManager::handleClientData(ClientConnection& client) {
    uint8_t buffer[4096];
    int received = recv(client.socket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);

    if (received > 0) {
        std::vector<uint8_t> data(buffer, buffer + received);
        NetworkPacket packet = NetworkPacket::deserialize(data);
        processClientPacket(client, packet);
    } else if (received == 0) {
        // Client disconnected gracefully
        client.socket = INVALID_SOCKET;
    }
}

void ServerNetworkManager::processClientPacket(ClientConnection& client, const NetworkPacket& packet) {
    // Add packet to incoming queue for game logic to process
    incomingPackets.push(packet);
}

void ServerNetworkManager::removeDisconnectedClients() {
    clients.erase(
        std::remove_if(clients.begin(), clients.end(),
            [](const ClientConnection& client) { return client.socket == INVALID_SOCKET; }),
        clients.end()
    );
}

void ServerNetworkManager::disconnectClient(uint32_t clientId) {
    for (auto& client : clients) {
        if (client.clientId == clientId) {
            closesocket(client.socket);
            client.socket = INVALID_SOCKET;
            break;
        }
    }
}

void ServerNetworkManager::broadcastPacket(const NetworkPacket& packet) {
    std::vector<uint8_t> data = packet.serialize();
    for (auto& client : clients) {
        if (client.socket != INVALID_SOCKET) {
            sendData(client.socket, data.data(), static_cast<int>(data.size()));
        }
    }
}

void ServerNetworkManager::sendToClient(uint32_t clientId, const NetworkPacket& packet) {
    for (auto& client : clients) {
        if (client.clientId == clientId && client.socket != INVALID_SOCKET) {
            std::vector<uint8_t> data = packet.serialize();
            sendData(client.socket, data.data(), static_cast<int>(data.size()));
            break;
        }
    }
}

void ServerNetworkManager::broadcastWorldState(const World* world) {
    NetworkPacket packet(PacketType::WORLD_STATE_DELTA);
    packet.writeInt32(world->currentGeneration);
    packet.writeFloat(world->timeOfDay);
    packet.writeFloat(world->weatherIntensity);

    // Send faction stats
    for (int i = 0; i < 3; ++i) {
        packet.writeInt32(world->factions[i]->getPopulation());
        packet.writeFloat(world->factions[i]->getTotalResources());
    }

    broadcastPacket(packet);
}

void ServerNetworkManager::broadcastAgentUpdate(const Agent* agent) {
    NetworkPacket packet(PacketType::AGENT_UPDATE);
    packet.writeVector3(agent->position);
    packet.writeUInt8(static_cast<uint8_t>(agent->faction));
    packet.writeUInt8(static_cast<uint8_t>(agent->role));
    packet.writeFloat(agent->health);
    packet.writeFloat(agent->energy);
    broadcastPacket(packet);
}

void ServerNetworkManager::broadcastBuildingUpdate(const Building* building) {
    NetworkPacket packet(PacketType::BUILDING_UPDATE);
    packet.writeVector3(building->position);
    packet.writeUInt8(static_cast<uint8_t>(building->type));
    packet.writeUInt8(static_cast<uint8_t>(building->faction));
    packet.writeFloat(building->health);
    packet.writeInt32(building->occupants);
    broadcastPacket(packet);
}

void ServerNetworkManager::broadcastCombatEvent(const Agent* attacker, const Agent* defender, float damage) {
    NetworkPacket packet(PacketType::COMBAT_EVENT);
    packet.writeVector3(attacker->position);
    packet.writeVector3(defender->position);
    packet.writeFloat(damage);
    broadcastPacket(packet);
}

void ServerNetworkManager::broadcastAgentSpawn(const Agent* agent) {
    NetworkPacket packet(PacketType::AGENT_SPAWN);
    packet.writeVector3(agent->position);
    packet.writeUInt8(static_cast<uint8_t>(agent->faction));
    packet.writeUInt8(static_cast<uint8_t>(agent->role));
    broadcastPacket(packet);
}

void ServerNetworkManager::broadcastAgentDeath(const Agent* agent) {
    NetworkPacket packet(PacketType::AGENT_DEATH);
    packet.writeVector3(agent->position);
    packet.writeUInt8(static_cast<uint8_t>(agent->faction));
    broadcastPacket(packet);
}

void ServerNetworkManager::shutdown() {
    for (auto& client : clients) {
        if (client.socket != INVALID_SOCKET) {
            closesocket(client.socket);
        }
    }
    clients.clear();
    NetworkManager::shutdown();
}

// ClientNetworkManager implementation

ClientNetworkManager::ClientNetworkManager(const std::string& server, int port)
    : serverAddress(server), serverPort(port), connected(false),
      reconnectTimer(0), clientId(0), faction(Faction::RED),
      lastPingTime(0), currentPing(0) {
    playerName = "Player";
}

ClientNetworkManager::~ClientNetworkManager() {
    shutdown();
}

bool ClientNetworkManager::initialize() {
    if (!initializeWinsock()) {
        return false;
    }
    initialized = true;
    return true;
}

bool ClientNetworkManager::connectToServer() {
    mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mainSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverAddress.c_str(), &serverAddr.sin_addr);

    if (connect(mainSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed" << std::endl;
        closesocket(mainSocket);
        mainSocket = INVALID_SOCKET;
        return false;
    }

    // Set to non-blocking
    u_long mode = 1;
    ioctlsocket(mainSocket, FIONBIO, &mode);

    connected = true;
    std::cout << "Connected to server at " << serverAddress << ":" << serverPort << std::endl;

    // Send connection request
    NetworkPacket packet(PacketType::CONNECT_REQUEST);
    packet.writeString(playerName);
    std::vector<uint8_t> data = packet.serialize();
    sendData(mainSocket, data.data(), static_cast<int>(data.size()));

    return true;
}

void ClientNetworkManager::update(float deltaTime) {
    if (!initialized) return;

    if (!connected) {
        reconnectTimer += deltaTime;
        if (reconnectTimer > 5.0f) {
            reconnectTimer = 0;
            connectToServer();
        }
        return;
    }

    // Handle incoming data
    handleServerData();

    // Send ping periodically
    lastPingTime += deltaTime;
    if (lastPingTime > 1.0f) {
        sendPing();
        lastPingTime = 0;
    }

    // Send queued packets
    while (!outgoingPackets.empty()) {
        NetworkPacket packet = outgoingPackets.front();
        outgoingPackets.pop();

        std::vector<uint8_t> data = packet.serialize();
        if (sendData(mainSocket, data.data(), static_cast<int>(data.size())) == SOCKET_ERROR) {
            connected = false;
            break;
        }
    }
}

void ClientNetworkManager::handleServerData() {
    uint8_t buffer[4096];
    int received = recv(mainSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);

    if (received > 0) {
        std::vector<uint8_t> data(buffer, buffer + received);
        NetworkPacket packet = NetworkPacket::deserialize(data);
        processServerPacket(packet);
    } else if (received == 0) {
        connected = false;
        std::cout << "Disconnected from server" << std::endl;
    }
}

void ClientNetworkManager::processServerPacket(const NetworkPacket& packet) {
    if (packet.type == PacketType::CONNECT_RESPONSE) {
        size_t offset = 0;
        clientId = packet.readUInt32(offset);
        std::cout << "Assigned client ID: " << clientId << std::endl;
    } else if (packet.type == PacketType::PONG) {
        // Calculate ping
        currentPing = lastPingTime * 1000.0f; // Convert to ms
    } else {
        // Queue for game logic processing
        incomingPackets.push(packet);
    }
}

void ClientNetworkManager::sendPing() {
    NetworkPacket packet(PacketType::PING);
    queuePacket(packet);
}

void ClientNetworkManager::disconnect() {
    if (connected) {
        NetworkPacket packet(PacketType::DISCONNECT);
        std::vector<uint8_t> data = packet.serialize();
        sendData(mainSocket, data.data(), static_cast<int>(data.size()));
    }
    connected = false;
}

void ClientNetworkManager::sendPlayerInput(const Vector3& clickPosition) {
    NetworkPacket packet(PacketType::PLAYER_INPUT);
    packet.writeVector3(clickPosition);
    queuePacket(packet);
}

void ClientNetworkManager::sendSelectAgent(uint32_t agentId) {
    NetworkPacket packet(PacketType::SELECT_AGENT);
    packet.writeUInt32(agentId);
    queuePacket(packet);
}

void ClientNetworkManager::sendSelectBuilding(uint32_t buildingId) {
    NetworkPacket packet(PacketType::SELECT_BUILDING);
    packet.writeUInt32(buildingId);
    queuePacket(packet);
}

void ClientNetworkManager::sendChangeRole(uint32_t agentId, Role newRole) {
    NetworkPacket packet(PacketType::CHANGE_ROLE);
    packet.writeUInt32(agentId);
    packet.writeUInt8(static_cast<uint8_t>(newRole));
    queuePacket(packet);
}

void ClientNetworkManager::sendBuildOrder(Building::Type type, const Vector3& position) {
    NetworkPacket packet(PacketType::BUILD_ORDER);
    packet.writeUInt8(static_cast<uint8_t>(type));
    packet.writeVector3(position);
    queuePacket(packet);
}

void ClientNetworkManager::sendChatMessage(const std::string& message) {
    NetworkPacket packet(PacketType::CHAT_MESSAGE);
    packet.writeString(message);
    queuePacket(packet);
}

void ClientNetworkManager::shutdown() {
    disconnect();
    NetworkManager::shutdown();
}
