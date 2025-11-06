#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include "AuthManager.h"
#include "LobbyManager.h"
#include <map>
#include <vector>
#include <algorithm>
#include <fstream>

// Friend Manager - handles friend requests, friend lists, and invites
class FriendManager {
public:
    FriendManager(AuthManager* authMgr, LobbyManager* lobbyMgr)
        : authManager(authMgr), lobbyManager(lobbyMgr) {
        loadFriendships();
    }

    ~FriendManager() {
        saveFriendships();
    }

    // Send friend request
    bool sendFriendRequest(uint64_t fromAccountId, const std::string& toUsername, std::string& errorMsg) {
        // Find target account
        Account* targetAccount = authManager->getAccountByUsername(toUsername);
        if (!targetAccount) {
            errorMsg = "User not found";
            return false;
        }

        uint64_t toAccountId = targetAccount->accountId;

        // Cannot friend yourself
        if (fromAccountId == toAccountId) {
            errorMsg = "Cannot add yourself as friend";
            return false;
        }

        // Check if already friends or pending
        Friend* existingFriend = getFriendship(fromAccountId, toAccountId);
        if (existingFriend) {
            if (existingFriend->status == FriendStatus::ACCEPTED) {
                errorMsg = "Already friends";
                return false;
            } else if (existingFriend->status == FriendStatus::PENDING) {
                errorMsg = "Friend request already sent";
                return false;
            } else if (existingFriend->status == FriendStatus::BLOCKED) {
                errorMsg = "Cannot send friend request";
                return false;
            }
        }

        // Create friend request
        Friend friendReq;
        friendReq.accountId = toAccountId;
        friendReq.username = toUsername;
        friendReq.status = FriendStatus::PENDING;
        friendReq.isOnline = false;
        friendReq.lobbyId = 0;
        friendReq.created = getCurrentTimestamp();

        // Add to both sides
        friendships[fromAccountId].push_back(friendReq);

        Friend reverseFriend;
        reverseFriend.accountId = fromAccountId;
        Account* fromAccount = authManager->getAccount(fromAccountId);
        if (fromAccount) {
            reverseFriend.username = fromAccount->username;
        }
        reverseFriend.status = FriendStatus::PENDING;
        reverseFriend.isOnline = false;
        reverseFriend.lobbyId = 0;
        reverseFriend.created = friendReq.created;
        friendships[toAccountId].push_back(reverseFriend);

        std::cout << "[FriendManager] Friend request sent from " << fromAccountId
                  << " to " << toUsername << std::endl;

        saveFriendships();
        return true;
    }

    // Accept friend request
    bool acceptFriendRequest(uint64_t accountId, uint64_t friendAccountId, std::string& errorMsg) {
        // Find friend request
        Friend* friendReq = getFriendship(accountId, friendAccountId);
        if (!friendReq) {
            errorMsg = "Friend request not found";
            return false;
        }

        if (friendReq->status != FriendStatus::PENDING) {
            errorMsg = "No pending friend request from this user";
            return false;
        }

        // Accept on both sides
        friendReq->status = FriendStatus::ACCEPTED;

        Friend* reverseFriend = getFriendship(friendAccountId, accountId);
        if (reverseFriend) {
            reverseFriend->status = FriendStatus::ACCEPTED;
        }

        std::cout << "[FriendManager] Friend request accepted: " << accountId
                  << " <-> " << friendAccountId << std::endl;

        saveFriendships();
        return true;
    }

    // Decline friend request
    bool declineFriendRequest(uint64_t accountId, uint64_t friendAccountId, std::string& errorMsg) {
        // Remove from both sides
        removeFriendship(accountId, friendAccountId);
        removeFriendship(friendAccountId, accountId);

        std::cout << "[FriendManager] Friend request declined: " << accountId
                  << " declined " << friendAccountId << std::endl;

        saveFriendships();
        return true;
    }

    // Remove friend
    bool removeFriend(uint64_t accountId, uint64_t friendAccountId, std::string& errorMsg) {
        // Remove from both sides
        removeFriendship(accountId, friendAccountId);
        removeFriendship(friendAccountId, accountId);

        std::cout << "[FriendManager] Friendship removed: " << accountId
                  << " <-> " << friendAccountId << std::endl;

        saveFriendships();
        return true;
    }

    // Get friend list
    std::vector<Friend> getFriendList(uint64_t accountId) {
        auto it = friendships.find(accountId);
        if (it != friendships.end()) {
            // Update online status and lobby info
            for (auto& friendEntry : it->second) {
                updateFriendStatus(friendEntry);
            }
            return it->second;
        }
        return std::vector<Friend>();
    }

    // Get accepted friends only
    std::vector<Friend> getAcceptedFriends(uint64_t accountId) {
        std::vector<Friend> result;
        auto friends = getFriendList(accountId);
        for (const auto& friendEntry : friends) {
            if (friendEntry.status == FriendStatus::ACCEPTED) {
                result.push_back(friendEntry);
            }
        }
        return result;
    }

    // Get pending friend requests
    std::vector<Friend> getPendingRequests(uint64_t accountId) {
        std::vector<Friend> result;
        auto friends = getFriendList(accountId);
        for (const auto& friendEntry : friends) {
            if (friendEntry.status == FriendStatus::PENDING) {
                result.push_back(friendEntry);
            }
        }
        return result;
    }

    // Invite friend to lobby
    bool inviteFriendToLobby(uint64_t accountId, uint64_t friendAccountId, std::string& errorMsg) {
        // Check if they are friends
        Friend* friendEntry = getFriendship(accountId, friendAccountId);
        if (!friendEntry || friendEntry->status != FriendStatus::ACCEPTED) {
            errorMsg = "Not friends with this user";
            return false;
        }

        // Check if sender is in a lobby
        Lobby* lobby = lobbyManager->getPlayerLobby(accountId);
        if (!lobby) {
            errorMsg = "You are not in a lobby";
            return false;
        }

        // Check if friend is already in a lobby
        if (lobbyManager->isPlayerInLobby(friendAccountId)) {
            errorMsg = "Friend is already in a lobby";
            return false;
        }

        // Check if lobby is full
        if (lobby->isFull()) {
            errorMsg = "Lobby is full";
            return false;
        }

        std::cout << "[FriendManager] Lobby invite sent from " << accountId
                  << " to " << friendAccountId << " (Lobby: " << lobby->lobbyId << ")" << std::endl;

        return true;
    }

    // Update online status
    void setOnlineStatus(uint64_t accountId, bool online) {
        // Update in all friendships where this account appears
        for (auto& pair : friendships) {
            for (auto& friendEntry : pair.second) {
                if (friendEntry.accountId == accountId) {
                    friendEntry.isOnline = online;
                }
            }
        }
    }

    // Save friendships to file
    void saveFriendships() {
        std::ofstream file("Server/friendships.dat");
        if (!file.is_open()) {
            std::cout << "[FriendManager] Failed to save friendships" << std::endl;
            return;
        }

        file << "FRIENDSHIPS_V1\n";
        file << friendships.size() << "\n";

        for (const auto& pair : friendships) {
            file << pair.first << "\n";  // accountId
            file << pair.second.size() << "\n";  // number of friends

            for (const auto& friendEntry : pair.second) {
                file << friendEntry.accountId << "\n";
                file << friendEntry.username << "\n";
                file << static_cast<int>(friendEntry.status) << "\n";
                file << friendEntry.created << "\n";
            }
        }

        file.close();
        std::cout << "[FriendManager] Saved friendships" << std::endl;
    }

    // Load friendships from file
    void loadFriendships() {
        std::ifstream file("Server/friendships.dat");
        if (!file.is_open()) {
            std::cout << "[FriendManager] No friendships file found, starting fresh" << std::endl;
            return;
        }

        std::string line;
        std::getline(file, line);  // Version
        if (line != "FRIENDSHIPS_V1") {
            std::cout << "[FriendManager] Invalid friendships file version" << std::endl;
            return;
        }

        int accountCount;
        file >> accountCount;
        file.ignore();

        for (int i = 0; i < accountCount; i++) {
            uint64_t accountId;
            file >> accountId;

            int friendCount;
            file >> friendCount;
            file.ignore();

            std::vector<Friend> friends;
            for (int j = 0; j < friendCount; j++) {
                Friend friendEntry;
                file >> friendEntry.accountId;
                file.ignore();
                std::getline(file, friendEntry.username);

                int status;
                file >> status;
                friendEntry.status = static_cast<FriendStatus>(status);

                file >> friendEntry.created;
                file.ignore();

                friendEntry.isOnline = false;
                friendEntry.lobbyId = 0;

                friends.push_back(friendEntry);
            }

            friendships[accountId] = friends;
        }

        file.close();
        std::cout << "[FriendManager] Loaded friendships" << std::endl;
    }

private:
    AuthManager* authManager;
    LobbyManager* lobbyManager;
    std::map<uint64_t, std::vector<Friend>> friendships;

    Friend* getFriendship(uint64_t accountId, uint64_t friendAccountId) {
        auto it = friendships.find(accountId);
        if (it != friendships.end()) {
            for (auto& friendEntry : it->second) {
                if (friendEntry.accountId == friendAccountId) {
                    return &friendEntry;
                }
            }
        }
        return nullptr;
    }

    void removeFriendship(uint64_t accountId, uint64_t friendAccountId) {
        auto it = friendships.find(accountId);
        if (it != friendships.end()) {
            auto friendIt = std::find_if(it->second.begin(), it->second.end(),
                [friendAccountId](const Friend& f) { return f.accountId == friendAccountId; });

            if (friendIt != it->second.end()) {
                it->second.erase(friendIt);
            }
        }
    }

    void updateFriendStatus(Friend& friendEntry) {
        // Check if online (has active session)
        uint64_t clientId;
        friendEntry.isOnline = authManager->getClientForAccount(friendEntry.accountId, clientId);

        // Check lobby status
        Lobby* lobby = lobbyManager->getPlayerLobby(friendEntry.accountId);
        friendEntry.lobbyId = lobby ? lobby->lobbyId : 0;
    }
};
