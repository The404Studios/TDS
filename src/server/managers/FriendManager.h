#pragma once
#include "../../common/NetworkProtocol.h"
#include "../../common/DataStructures.h"
#include "AuthManager.h"
#include "LobbyManager.h"
#include <map>
#include <vector>
#include <string>

// Friend Manager - handles friend requests, friend lists, and invites
class FriendManager {
public:
    FriendManager(AuthManager* authMgr, LobbyManager* lobbyMgr);
    ~FriendManager();

    // Send friend request
    bool sendFriendRequest(uint64_t fromAccountId, const std::string& toUsername, std::string& errorMsg);

    // Accept friend request
    bool acceptFriendRequest(uint64_t accountId, uint64_t friendAccountId, std::string& errorMsg);

    // Decline friend request
    bool declineFriendRequest(uint64_t accountId, uint64_t friendAccountId, std::string& errorMsg);

    // Remove friend
    bool removeFriend(uint64_t accountId, uint64_t friendAccountId, std::string& errorMsg);

    // Get friend list
    std::vector<Friend> getFriendList(uint64_t accountId);

    // Get accepted friends only
    std::vector<Friend> getAcceptedFriends(uint64_t accountId);

    // Get pending friend requests
    std::vector<Friend> getPendingRequests(uint64_t accountId);

    // Invite friend to lobby
    bool inviteFriendToLobby(uint64_t accountId, uint64_t friendAccountId, std::string& errorMsg);

    // Update online status
    void setOnlineStatus(uint64_t accountId, bool online);

    // Save friendships to file
    void saveFriendships();

    // Load friendships from file
    void loadFriendships();

private:
    AuthManager* authManager;
    LobbyManager* lobbyManager;
    std::map<uint64_t, std::vector<Friend>> friendships;

    Friend* getFriendship(uint64_t accountId, uint64_t friendAccountId);
    void removeFriendship(uint64_t accountId, uint64_t friendAccountId);
    void updateFriendStatus(Friend& friendEntry);
};
