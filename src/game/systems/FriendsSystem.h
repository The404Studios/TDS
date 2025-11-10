#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

/**
 * GameFriend - Represents a friend connection (client-side)
 */
struct GameFriend {
    uint64_t accountId;
    std::string username;

    enum class Status {
        OFFLINE,
        ONLINE,
        IN_MENU,
        IN_LOBBY,
        IN_RAID
    };
    Status status;

    // Party info (if in a party)
    uint64_t partyId;
    bool isPartyLeader;

    GameFriend() : accountId(0), status(Status::OFFLINE), partyId(0), isPartyLeader(false) {}
};

/**
 * GameFriendRequest - Pending friend request (client-side)
 */
struct GameFriendRequest {
    uint64_t fromAccountId;
    uint64_t toAccountId;
    std::string fromUsername;
    std::string toUsername;
    uint64_t timestamp;

    GameFriendRequest() : fromAccountId(0), toAccountId(0), timestamp(0) {}
};

/**
 * FriendsSystem - Manages friends list and social features
 */
class FriendsSystem {
public:
    FriendsSystem();
    ~FriendsSystem();

    // Friend management
    void addFriend(uint64_t accountId, const std::string& username);
    void removeFriend(uint64_t accountId);
    bool isFriend(uint64_t accountId) const;
    GameFriend* getFriend(uint64_t accountId);
    const std::vector<GameFriend>& getFriendsList() const { return friends; }

    // Friend requests
    void sendFriendRequest(uint64_t toAccountId, const std::string& toUsername);
    void acceptFriendRequest(uint64_t fromAccountId);
    void declineFriendRequest(uint64_t fromAccountId);
    const std::vector<GameFriendRequest>& getPendingRequests() const { return pendingRequests; }
    const std::vector<GameFriendRequest>& getSentRequests() const { return sentRequests; }

    // Status updates
    void updateFriendStatus(uint64_t accountId, GameFriend::Status status);
    void updateFriendParty(uint64_t accountId, uint64_t partyId, bool isLeader);

    // Filtering
    std::vector<GameFriend*> getOnlineFriends();
    std::vector<GameFriend*> getFriendsInMenu();
    std::vector<GameFriend*> getFriendsInLobby();

    // Invites
    void inviteToParty(uint64_t friendAccountId);

    // Callbacks
    void setOnFriendRequestReceived(std::function<void(const GameFriendRequest&)> callback) {
        onFriendRequestReceived = callback;
    }
    void setOnFriendAdded(std::function<void(const GameFriend&)> callback) {
        onFriendAdded = callback;
    }
    void setOnFriendRemoved(std::function<void(uint64_t)> callback) {
        onFriendRemoved = callback;
    }
    void setOnFriendStatusChanged(std::function<void(const GameFriend&)> callback) {
        onFriendStatusChanged = callback;
    }
    void setOnPartyInviteReceived(std::function<void(uint64_t, const std::string&)> callback) {
        onPartyInviteReceived = callback;
    }

private:
    std::vector<GameFriend> friends;
    std::vector<GameFriendRequest> pendingRequests;  // Requests we've received
    std::vector<GameFriendRequest> sentRequests;     // Requests we've sent

    // Callbacks
    std::function<void(const GameFriendRequest&)> onFriendRequestReceived;
    std::function<void(const GameFriend&)> onFriendAdded;
    std::function<void(uint64_t)> onFriendRemoved;
    std::function<void(const GameFriend&)> onFriendStatusChanged;
    std::function<void(uint64_t, const std::string&)> onPartyInviteReceived;
};
