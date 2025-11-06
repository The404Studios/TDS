#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include <map>
#include <vector>
#include <string>

// Lobby Manager - handles lobby creation, joining, and party management
class LobbyManager {
public:
    LobbyManager();

    // Create new lobby
    bool createLobby(uint64_t ownerAccountId, const std::string& lobbyName,
                     int maxPlayers, bool isPrivate, uint64_t& outLobbyId,
                     std::string& errorMsg);

    // Join lobby
    bool joinLobby(uint64_t accountId, uint64_t lobbyId, std::string& errorMsg);

    // Leave lobby
    bool leaveLobby(uint64_t accountId, std::string& errorMsg);

    // Kick player from lobby
    bool kickPlayer(uint64_t ownerAccountId, uint64_t targetAccountId, std::string& errorMsg);

    // Set ready status
    bool setReady(uint64_t accountId, bool ready, std::string& errorMsg);

    // Start queue
    bool startQueue(uint64_t accountId, std::string& errorMsg);

    // Stop queue
    bool stopQueue(uint64_t accountId, std::string& errorMsg);

    // Get lobby
    Lobby* getLobby(uint64_t lobbyId);

    // Get player's lobby
    Lobby* getPlayerLobby(uint64_t accountId);

    // Check if player is in lobby
    bool isPlayerInLobby(uint64_t accountId) const;

    // Get queued lobbies
    const std::vector<uint64_t>& getQueuedLobbies() const;

    // Set lobby state
    void setLobbyState(uint64_t lobbyId, LobbyState state);

    // Remove lobby
    void removeLobby(uint64_t lobbyId);

    // Get all lobbies
    const std::map<uint64_t, Lobby>& getAllLobbies() const;

private:
    std::map<uint64_t, Lobby> lobbies;
    std::map<uint64_t, uint64_t> playerLobbies;  // accountId -> lobbyId
    std::vector<uint64_t> queuedLobbies;
    uint64_t nextLobbyId;
};
