#include "LobbyManager.h"
#include <iostream>
#include <algorithm>

LobbyManager::LobbyManager() : nextLobbyId(1) {}

bool LobbyManager::createLobby(uint64_t ownerAccountId, const std::string& lobbyName,
                               int maxPlayers, bool isPrivate, uint64_t& outLobbyId,
                               std::string& errorMsg) {
    // Check if player is already in a lobby
    if (isPlayerInLobby(ownerAccountId)) {
        errorMsg = "You are already in a lobby";
        return false;
    }

    // Validate max players
    if (maxPlayers < 1 || maxPlayers > 5) {
        errorMsg = "Max players must be 1-5";
        return false;
    }

    // Create lobby
    Lobby lobby;
    lobby.lobbyId = nextLobbyId++;
    lobby.ownerId = ownerAccountId;
    lobby.lobbyName = lobbyName.empty() ? "Lobby" : lobbyName;
    lobby.maxPlayers = maxPlayers;
    lobby.isPrivate = isPrivate;
    lobby.state = LobbyState::WAITING;
    lobby.created = getCurrentTimestamp();

    // Add owner as first member
    LobbyMember owner;
    owner.accountId = ownerAccountId;
    owner.isReady = false;
    owner.isOwner = true;
    lobby.members.push_back(owner);

    lobbies[lobby.lobbyId] = lobby;
    playerLobbies[ownerAccountId] = lobby.lobbyId;

    outLobbyId = lobby.lobbyId;

    std::cout << "[LobbyManager] Lobby created: " << lobby.lobbyName
              << " (ID: " << lobby.lobbyId << ")" << std::endl;

    return true;
}

bool LobbyManager::joinLobby(uint64_t accountId, uint64_t lobbyId, std::string& errorMsg) {
    // Check if player is already in a lobby
    if (isPlayerInLobby(accountId)) {
        errorMsg = "You are already in a lobby";
        return false;
    }

    // Find lobby
    auto it = lobbies.find(lobbyId);
    if (it == lobbies.end()) {
        errorMsg = "Lobby not found";
        return false;
    }

    Lobby& lobby = it->second;

    // Check if lobby is full
    if (lobby.isFull()) {
        errorMsg = "Lobby is full";
        return false;
    }

    // Check if lobby is in match
    if (lobby.state == LobbyState::IN_MATCH) {
        errorMsg = "Lobby is in a match";
        return false;
    }

    // Add member
    LobbyMember member;
    member.accountId = accountId;
    member.isReady = false;
    member.isOwner = false;
    lobby.members.push_back(member);

    playerLobbies[accountId] = lobbyId;

    std::cout << "[LobbyManager] Player " << accountId << " joined lobby " << lobbyId << std::endl;

    return true;
}

bool LobbyManager::leaveLobby(uint64_t accountId, std::string& errorMsg) {
    // Find player's lobby
    auto it = playerLobbies.find(accountId);
    if (it == playerLobbies.end()) {
        errorMsg = "You are not in a lobby";
        return false;
    }

    uint64_t lobbyId = it->second;
    Lobby& lobby = lobbies[lobbyId];

    // Remove member
    auto memberIt = std::find_if(lobby.members.begin(), lobby.members.end(),
        [accountId](const LobbyMember& m) { return m.accountId == accountId; });

    if (memberIt != lobby.members.end()) {
        bool wasOwner = memberIt->isOwner;
        lobby.members.erase(memberIt);

        // If owner left, transfer ownership or delete lobby
        if (wasOwner) {
            if (!lobby.members.empty()) {
                lobby.members[0].isOwner = true;
                lobby.ownerId = lobby.members[0].accountId;
                std::cout << "[LobbyManager] Ownership transferred in lobby " << lobbyId << std::endl;
            } else {
                // Delete empty lobby
                lobbies.erase(lobbyId);
                std::cout << "[LobbyManager] Lobby " << lobbyId << " deleted (empty)" << std::endl;
            }
        }
    }

    playerLobbies.erase(accountId);

    std::cout << "[LobbyManager] Player " << accountId << " left lobby " << lobbyId << std::endl;

    return true;
}

bool LobbyManager::kickPlayer(uint64_t ownerAccountId, uint64_t targetAccountId, std::string& errorMsg) {
    // Find owner's lobby
    auto it = playerLobbies.find(ownerAccountId);
    if (it == playerLobbies.end()) {
        errorMsg = "You are not in a lobby";
        return false;
    }

    uint64_t lobbyId = it->second;
    Lobby& lobby = lobbies[lobbyId];

    // Check if requester is owner
    if (lobby.ownerId != ownerAccountId) {
        errorMsg = "Only the owner can kick players";
        return false;
    }

    // Cannot kick self
    if (targetAccountId == ownerAccountId) {
        errorMsg = "Cannot kick yourself";
        return false;
    }

    // Remove target
    auto memberIt = std::find_if(lobby.members.begin(), lobby.members.end(),
        [targetAccountId](const LobbyMember& m) { return m.accountId == targetAccountId; });

    if (memberIt != lobby.members.end()) {
        lobby.members.erase(memberIt);
        playerLobbies.erase(targetAccountId);

        std::cout << "[LobbyManager] Player " << targetAccountId << " kicked from lobby " << lobbyId << std::endl;
        return true;
    }

    errorMsg = "Player not in lobby";
    return false;
}

bool LobbyManager::setReady(uint64_t accountId, bool ready, std::string& errorMsg) {
    // Find player's lobby
    auto it = playerLobbies.find(accountId);
    if (it == playerLobbies.end()) {
        errorMsg = "You are not in a lobby";
        return false;
    }

    uint64_t lobbyId = it->second;
    Lobby& lobby = lobbies[lobbyId];

    // Find member
    LobbyMember* member = lobby.findMember(accountId);
    if (member) {
        member->isReady = ready;

        // Check if all ready
        if (lobby.allReady() && lobby.state == LobbyState::WAITING) {
            lobby.state = LobbyState::READY;
            std::cout << "[LobbyManager] Lobby " << lobbyId << " is ready!" << std::endl;
        } else if (!lobby.allReady() && lobby.state == LobbyState::READY) {
            lobby.state = LobbyState::WAITING;
        }

        return true;
    }

    errorMsg = "Player not found in lobby";
    return false;
}

bool LobbyManager::startQueue(uint64_t accountId, std::string& errorMsg) {
    // Find player's lobby
    auto it = playerLobbies.find(accountId);
    if (it == playerLobbies.end()) {
        errorMsg = "You are not in a lobby";
        return false;
    }

    uint64_t lobbyId = it->second;
    Lobby& lobby = lobbies[lobbyId];

    // Check if requester is owner
    if (lobby.ownerId != accountId) {
        errorMsg = "Only the owner can start the queue";
        return false;
    }

    // Check if all ready
    if (!lobby.allReady()) {
        errorMsg = "Not all players are ready";
        return false;
    }

    // Start queue
    lobby.state = LobbyState::IN_QUEUE;
    queuedLobbies.push_back(lobbyId);

    std::cout << "[LobbyManager] Lobby " << lobbyId << " entered queue" << std::endl;

    return true;
}

bool LobbyManager::stopQueue(uint64_t accountId, std::string& errorMsg) {
    // Find player's lobby
    auto it = playerLobbies.find(accountId);
    if (it == playerLobbies.end()) {
        errorMsg = "You are not in a lobby";
        return false;
    }

    uint64_t lobbyId = it->second;
    Lobby& lobby = lobbies[lobbyId];

    // Check if in queue
    if (lobby.state != LobbyState::IN_QUEUE) {
        errorMsg = "Lobby is not in queue";
        return false;
    }

    // Remove from queue
    auto queueIt = std::find(queuedLobbies.begin(), queuedLobbies.end(), lobbyId);
    if (queueIt != queuedLobbies.end()) {
        queuedLobbies.erase(queueIt);
    }

    lobby.state = LobbyState::READY;

    std::cout << "[LobbyManager] Lobby " << lobbyId << " left queue" << std::endl;

    return true;
}

Lobby* LobbyManager::getLobby(uint64_t lobbyId) {
    auto it = lobbies.find(lobbyId);
    if (it != lobbies.end()) {
        return &it->second;
    }
    return nullptr;
}

Lobby* LobbyManager::getPlayerLobby(uint64_t accountId) {
    auto it = playerLobbies.find(accountId);
    if (it != playerLobbies.end()) {
        return getLobby(it->second);
    }
    return nullptr;
}

bool LobbyManager::isPlayerInLobby(uint64_t accountId) const {
    return playerLobbies.find(accountId) != playerLobbies.end();
}

const std::vector<uint64_t>& LobbyManager::getQueuedLobbies() const {
    return queuedLobbies;
}

void LobbyManager::setLobbyState(uint64_t lobbyId, LobbyState state) {
    auto it = lobbies.find(lobbyId);
    if (it != lobbies.end()) {
        it->second.state = state;
    }
}

void LobbyManager::removeLobby(uint64_t lobbyId) {
    auto it = lobbies.find(lobbyId);
    if (it != lobbies.end()) {
        // Remove all player mappings
        for (const auto& member : it->second.members) {
            playerLobbies.erase(member.accountId);
        }

        // Remove from queue if present
        auto queueIt = std::find(queuedLobbies.begin(), queuedLobbies.end(), lobbyId);
        if (queueIt != queuedLobbies.end()) {
            queuedLobbies.erase(queueIt);
        }

        lobbies.erase(it);
        std::cout << "[LobbyManager] Lobby " << lobbyId << " removed" << std::endl;
    }
}

const std::map<uint64_t, Lobby>& LobbyManager::getAllLobbies() const {
    return lobbies;
}
