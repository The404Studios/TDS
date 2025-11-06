#include "network/NetworkServer.h"
#include "managers/AuthManager.h"
#include "managers/LobbyManager.h"
#include "managers/FriendManager.h"
#include "managers/MatchManager.h"
#include "managers/PersistenceManager.h"
#include "managers/MerchantManager.h"
#include "../common/ItemDatabase.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>

// Global managers
NetworkServer* g_networkServer = nullptr;
AuthManager* g_authManager = nullptr;
LobbyManager* g_lobbyManager = nullptr;
FriendManager* g_friendManager = nullptr;
MatchManager* g_matchManager = nullptr;
PersistenceManager* g_persistenceManager = nullptr;
MerchantManager* g_merchantManager = nullptr;

// Forward declarations
void processPackets();
void handleLoginRequest(uint64_t clientId, const std::vector<uint8_t>& payload);
void handleRegisterRequest(uint64_t clientId, const std::vector<uint8_t>& payload);
void handleLobbyCreate(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload);
void handleLobbyJoin(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload);
void handleLobbyLeave(uint64_t clientId, uint64_t sessionToken);
void handleLobbyReady(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload);
void handleLobbyStartQueue(uint64_t clientId, uint64_t sessionToken);
void handleFriendRequest(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload);
void handleFriendAccept(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload);
void handleMerchantBuy(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload);
void handleMerchantSell(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload);
void updateMatchmaking();
void sendLobbyUpdate(uint64_t lobbyId);

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  EXTRACTION SHOOTER - Dedicated Server " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Initialize item database
    auto& itemDb = ItemDatabase::getInstance();
    std::cout << "[Server] Item database initialized" << std::endl;

    // Create managers
    g_networkServer = new NetworkServer();
    g_authManager = new AuthManager();
    g_persistenceManager = new PersistenceManager();
    g_lobbyManager = new LobbyManager();
    g_matchManager = new MatchManager();
    g_friendManager = new FriendManager(g_authManager, g_lobbyManager);
    g_merchantManager = new MerchantManager(g_persistenceManager);

    // Start server
    if (!g_networkServer->start(7777)) {
        std::cout << "[Server] Failed to start server!" << std::endl;
        return 1;
    }

    std::cout << "[Server] Server is running on port 7777" << std::endl;
    std::cout << "[Server] Press Ctrl+C to shutdown" << std::endl;
    std::cout << std::endl;

    // Main loop
    bool running = true;
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Update network
        g_networkServer->update();

        // Process packets
        processPackets();

        // Update matchmaking
        updateMatchmaking();

        // Update matches
        g_matchManager->update();

        // Sleep to avoid 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
    }

    // Cleanup
    std::cout << "[Server] Shutting down..." << std::endl;

    delete g_merchantManager;
    delete g_friendManager;
    delete g_matchManager;
    delete g_lobbyManager;
    delete g_persistenceManager;
    delete g_authManager;
    delete g_networkServer;

    std::cout << "[Server] Shutdown complete" << std::endl;

    return 0;
}

void processPackets() {
    auto packets = g_networkServer->getReceivedPackets();

    for (const auto& packet : packets) {
        switch (packet.type) {
            case PacketType::LOGIN_REQUEST:
                handleLoginRequest(packet.clientId, packet.payload);
                break;

            case PacketType::REGISTER_REQUEST:
                handleRegisterRequest(packet.clientId, packet.payload);
                break;

            case PacketType::LOBBY_CREATE:
                handleLobbyCreate(packet.clientId, packet.sessionToken, packet.payload);
                break;

            case PacketType::LOBBY_JOIN:
                handleLobbyJoin(packet.clientId, packet.sessionToken, packet.payload);
                break;

            case PacketType::LOBBY_LEAVE:
                handleLobbyLeave(packet.clientId, packet.sessionToken);
                break;

            case PacketType::LOBBY_READY:
                handleLobbyReady(packet.clientId, packet.sessionToken, packet.payload);
                break;

            case PacketType::LOBBY_START_QUEUE:
                handleLobbyStartQueue(packet.clientId, packet.sessionToken);
                break;

            case PacketType::FRIEND_REQUEST:
                handleFriendRequest(packet.clientId, packet.sessionToken, packet.payload);
                break;

            case PacketType::FRIEND_ACCEPT:
                handleFriendAccept(packet.clientId, packet.sessionToken, packet.payload);
                break;

            case PacketType::MERCHANT_BUY:
                handleMerchantBuy(packet.clientId, packet.sessionToken, packet.payload);
                break;

            case PacketType::MERCHANT_SELL:
                handleMerchantSell(packet.clientId, packet.sessionToken, packet.payload);
                break;

            case PacketType::DISCONNECT:
                g_authManager->handleClientDisconnect(packet.clientId);
                break;

            default:
                std::cout << "[Server] Unhandled packet type: " << static_cast<int>(packet.type) << std::endl;
                break;
        }
    }
}

void handleLoginRequest(uint64_t clientId, const std::vector<uint8_t>& payload) {
    if (payload.size() < sizeof(LoginRequest)) {
        std::cout << "[Server] Invalid LOGIN_REQUEST payload size" << std::endl;
        return;
    }

    LoginRequest req;
    memcpy(&req, payload.data(), sizeof(LoginRequest));

    std::string username(req.username);
    std::string passwordHash(req.passwordHash);

    LoginResponse resp;
    uint64_t accountId, sessionToken;

    std::string errorMsg;
    if (g_authManager->login(username, passwordHash, clientId, accountId, sessionToken, errorMsg)) {
        resp.success = true;
        resp.accountId = accountId;
        resp.sessionToken = sessionToken;
        strncpy_s(resp.errorMessage, "", sizeof(resp.errorMessage));

        // Load or create player data
        if (!g_persistenceManager->getPlayerData(accountId)) {
            g_persistenceManager->createPlayerData(accountId, username);
        }

        std::cout << "[Server] Login successful: " << username << std::endl;
    } else {
        resp.success = false;
        resp.accountId = 0;
        resp.sessionToken = 0;
        strncpy_s(resp.errorMessage, errorMsg.c_str(), sizeof(resp.errorMessage));

        std::cout << "[Server] Login failed: " << errorMsg << std::endl;
    }

    g_networkServer->sendPacket(clientId, PacketType::LOGIN_RESPONSE, &resp, sizeof(resp));
}

void handleRegisterRequest(uint64_t clientId, const std::vector<uint8_t>& payload) {
    if (payload.size() < sizeof(RegisterRequest)) {
        std::cout << "[Server] Invalid REGISTER_REQUEST payload size" << std::endl;
        return;
    }

    RegisterRequest req;
    memcpy(&req, payload.data(), sizeof(RegisterRequest));

    std::string username(req.username);
    std::string passwordHash(req.passwordHash);
    std::string email(req.email);

    RegisterResponse resp;
    uint64_t accountId;

    std::string errorMsg;
    if (g_authManager->registerAccount(username, passwordHash, email, accountId, errorMsg)) {
        resp.success = true;
        resp.accountId = accountId;
        strncpy_s(resp.errorMessage, "", sizeof(resp.errorMessage));

        // Create player data
        g_persistenceManager->createPlayerData(accountId, username);

        std::cout << "[Server] Registration successful: " << username << std::endl;
    } else {
        resp.success = false;
        resp.accountId = 0;
        strncpy_s(resp.errorMessage, errorMsg.c_str(), sizeof(resp.errorMessage));

        std::cout << "[Server] Registration failed: " << errorMsg << std::endl;
    }

    g_networkServer->sendPacket(clientId, PacketType::REGISTER_RESPONSE, &resp, sizeof(resp));
}

void handleLobbyCreate(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload) {
    // Validate session
    uint64_t accountId;
    if (!g_authManager->validateSession(sessionToken, accountId)) {
        ErrorResponse err;
        err.errorCode = 403;
        strncpy_s(err.errorMessage, "Invalid session", sizeof(err.errorMessage));
        g_networkServer->sendPacket(clientId, PacketType::ERROR_RESPONSE, &err, sizeof(err));
        return;
    }

    if (payload.size() < sizeof(LobbyCreateRequest)) {
        std::cout << "[Server] Invalid LOBBY_CREATE payload size" << std::endl;
        return;
    }

    LobbyCreateRequest req;
    memcpy(&req, payload.data(), sizeof(LobbyCreateRequest));

    LobbyCreateResponse resp;
    uint64_t lobbyId;
    std::string errorMsg;

    if (g_lobbyManager->createLobby(accountId, req.lobbyName, req.maxPlayers, req.isPrivate, lobbyId, errorMsg)) {
        resp.success = true;
        resp.lobbyId = lobbyId;
        strncpy_s(resp.errorMessage, "", sizeof(resp.errorMessage));

        // Send lobby update
        sendLobbyUpdate(lobbyId);
    } else {
        resp.success = false;
        resp.lobbyId = 0;
        strncpy_s(resp.errorMessage, errorMsg.c_str(), sizeof(resp.errorMessage));
    }

    g_networkServer->sendPacket(clientId, PacketType::LOBBY_CREATE_RESPONSE, &resp, sizeof(resp));
}

void handleLobbyJoin(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload) {
    // Validate session
    uint64_t accountId;
    if (!g_authManager->validateSession(sessionToken, accountId)) {
        return;
    }

    if (payload.size() < sizeof(LobbyJoinRequest)) {
        return;
    }

    LobbyJoinRequest req;
    memcpy(&req, payload.data(), sizeof(LobbyJoinRequest));

    LobbyJoinResponse resp;
    std::string errorMsg;

    if (g_lobbyManager->joinLobby(accountId, req.lobbyId, errorMsg)) {
        resp.success = true;
        resp.lobbyId = req.lobbyId;
        strncpy_s(resp.errorMessage, "", sizeof(resp.errorMessage));

        // Send lobby update to all members
        sendLobbyUpdate(req.lobbyId);
    } else {
        resp.success = false;
        resp.lobbyId = 0;
        strncpy_s(resp.errorMessage, errorMsg.c_str(), sizeof(resp.errorMessage));
    }

    g_networkServer->sendPacket(clientId, PacketType::LOBBY_JOIN_RESPONSE, &resp, sizeof(resp));
}

void handleLobbyLeave(uint64_t clientId, uint64_t sessionToken) {
    // Validate session
    uint64_t accountId;
    if (!g_authManager->validateSession(sessionToken, accountId)) {
        return;
    }

    Lobby* lobby = g_lobbyManager->getPlayerLobby(accountId);
    uint64_t lobbyId = lobby ? lobby->lobbyId : 0;

    std::string errorMsg;
    g_lobbyManager->leaveLobby(accountId, errorMsg);

    // Send lobby update to remaining members
    if (lobbyId != 0) {
        sendLobbyUpdate(lobbyId);
    }
}

void handleLobbyReady(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload) {
    // Validate session
    uint64_t accountId;
    if (!g_authManager->validateSession(sessionToken, accountId)) {
        return;
    }

    if (payload.size() < sizeof(LobbyReady)) {
        return;
    }

    LobbyReady req;
    memcpy(&req, payload.data(), sizeof(LobbyReady));

    std::string errorMsg;
    g_lobbyManager->setReady(accountId, req.ready, errorMsg);

    // Send lobby update
    Lobby* lobby = g_lobbyManager->getPlayerLobby(accountId);
    if (lobby) {
        sendLobbyUpdate(lobby->lobbyId);
    }
}

void handleLobbyStartQueue(uint64_t clientId, uint64_t sessionToken) {
    // Validate session
    uint64_t accountId;
    if (!g_authManager->validateSession(sessionToken, accountId)) {
        return;
    }

    std::string errorMsg;
    if (g_lobbyManager->startQueue(accountId, errorMsg)) {
        Lobby* lobby = g_lobbyManager->getPlayerLobby(accountId);
        if (lobby) {
            sendLobbyUpdate(lobby->lobbyId);
        }
    }
}

void handleFriendRequest(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload) {
    // Validate session
    uint64_t accountId;
    if (!g_authManager->validateSession(sessionToken, accountId)) {
        return;
    }

    if (payload.size() < sizeof(FriendRequest)) {
        return;
    }

    FriendRequest req;
    memcpy(&req, payload.data(), sizeof(FriendRequest));

    std::string errorMsg;
    g_friendManager->sendFriendRequest(accountId, req.targetUsername, errorMsg);
}

void handleFriendAccept(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload) {
    // Validate session
    uint64_t accountId;
    if (!g_authManager->validateSession(sessionToken, accountId)) {
        return;
    }

    if (payload.size() < sizeof(FriendAccept)) {
        return;
    }

    FriendAccept req;
    memcpy(&req, payload.data(), sizeof(FriendAccept));

    std::string errorMsg;
    g_friendManager->acceptFriendRequest(accountId, req.friendAccountId, errorMsg);
}

void handleMerchantBuy(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload) {
    // Validate session
    uint64_t accountId;
    if (!g_authManager->validateSession(sessionToken, accountId)) {
        return;
    }

    if (payload.size() < sizeof(MerchantBuy)) {
        return;
    }

    MerchantBuy req;
    memcpy(&req, payload.data(), sizeof(MerchantBuy));

    MerchantTransactionResponse resp;
    std::string errorMsg;

    // Convert itemId to string (simplified)
    std::string itemId = "ak74";  // TODO: Proper ID mapping

    if (g_merchantManager->buyItem(accountId, static_cast<MerchantType>(req.merchantId), itemId, req.quantity, errorMsg)) {
        resp.success = true;
        PlayerData* playerData = g_persistenceManager->getPlayerData(accountId);
        resp.newBalance = playerData ? playerData->stats.roubles : 0;
        strncpy_s(resp.errorMessage, "", sizeof(resp.errorMessage));
    } else {
        resp.success = false;
        resp.newBalance = 0;
        strncpy_s(resp.errorMessage, errorMsg.c_str(), sizeof(resp.errorMessage));
    }

    g_networkServer->sendPacket(clientId, PacketType::MERCHANT_TRANSACTION_RESPONSE, &resp, sizeof(resp));
}

void handleMerchantSell(uint64_t clientId, uint64_t sessionToken, const std::vector<uint8_t>& payload) {
    // Similar to handleMerchantBuy
    // Implementation omitted for brevity
}

void updateMatchmaking() {
    // Get queued lobbies
    const auto& queuedLobbies = g_lobbyManager->getQueuedLobbies();

    // For now, create a match for each queued lobby immediately
    for (uint64_t lobbyId : queuedLobbies) {
        Lobby* lobby = g_lobbyManager->getLobby(lobbyId);
        if (!lobby) continue;

        // Create match
        uint64_t matchId;
        if (g_matchManager->createMatch(lobby->members, "Factory", matchId)) {
            // Update lobby state
            g_lobbyManager->setLobbyState(lobbyId, LobbyState::IN_MATCH);

            // Send match found notification to all players
            MatchFound matchFound;
            matchFound.matchId = matchId;
            strncpy_s(matchFound.mapName, "Factory", sizeof(matchFound.mapName));

            for (const auto& member : lobby->members) {
                uint64_t clientId;
                if (g_authManager->getClientForAccount(member.accountId, clientId)) {
                    g_networkServer->sendPacket(clientId, PacketType::MATCH_FOUND, &matchFound, sizeof(matchFound));
                }
            }

            std::cout << "[Server] Match created for lobby " << lobbyId << std::endl;
        }
    }
}

void sendLobbyUpdate(uint64_t lobbyId) {
    Lobby* lobby = g_lobbyManager->getLobby(lobbyId);
    if (!lobby) return;

    LobbyUpdate update;
    update.lobbyId = lobbyId;
    update.memberCount = static_cast<uint8_t>(lobby->members.size());
    update.inQueue = (lobby->state == LobbyState::IN_QUEUE);

    // Copy members
    for (size_t i = 0; i < lobby->members.size() && i < 5; i++) {
        const auto& member = lobby->members[i];
        update.members[i].accountId = member.accountId;
        strncpy_s(update.members[i].username, member.username.c_str(), sizeof(update.members[i].username));
        update.members[i].isReady = member.isReady;
        update.members[i].isOwner = member.isOwner;
    }

    // Send to all members
    for (const auto& member : lobby->members) {
        uint64_t clientId;
        if (g_authManager->getClientForAccount(member.accountId, clientId)) {
            g_networkServer->sendPacket(clientId, PacketType::LOBBY_UPDATE, &update, sizeof(update));
        }
    }
}
