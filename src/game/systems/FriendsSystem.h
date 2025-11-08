#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

/**
 * Friend - Represents a friend connection
 */
struct Friend {
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

    Friend() : accountId(0), status(Status::OFFLINE), partyId(0), isPartyLeader(false) {}
};

/**
 * FriendRequest - Pending friend request
 */
struct FriendRequest {
    uint64_t fromAccountId;
    uint64_t toAccountId;
    std::string fromUsername;
    std::string toUsername;
    uint64_t timestamp;

    FriendRequest() : fromAccountId(0), toAccountId(0), timestamp(0) {}
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
    Friend* getFriend(uint64_t accountId);
    const std::vector<Friend>& getFriendsList() const { return friends; }

    // Friend requests
    void sendFriendRequest(uint64_t toAccountId, const std::string& toUsername);
    void acceptFriendRequest(uint64_t fromAccountId);
    void declineFriendRequest(uint64_t fromAccountId);
    const std::vector<FriendRequest>& getPendingRequests() const { return pendingRequests; }
    const std::vector<FriendRequest>& getSentRequests() const { return sentRequests; }

    // Status updates
    void updateFriendStatus(uint64_t accountId, Friend::Status status);
    void updateFriendParty(uint64_t accountId, uint64_t partyId, bool isLeader);

    // Filtering
    std::vector<Friend*> getOnlineFriends();
    std::vector<Friend*> getFriendsInMenu();
    std::vector<Friend*> getFriendsInLobby();

    // Invites
    void inviteToParty(uint64_t friendAccountId);

    // Callbacks
    void setOnFriendRequestReceived(std::function<void(const FriendRequest&)> callback) {
        onFriendRequestReceived = callback;
    }
    void setOnFriendAdded(std::function<void(const Friend&)> callback) {
        onFriendAdded = callback;
    }
    void setOnFriendRemoved(std::function<void(uint64_t)> callback) {
        onFriendRemoved = callback;
    }
    void setOnFriendStatusChanged(std::function<void(const Friend&)> callback) {
        onFriendStatusChanged = callback;
    }
    void setOnPartyInviteReceived(std::function<void(uint64_t, const std::string&)> callback) {
        onPartyInviteReceived = callback;
    }

private:
    std::vector<Friend> friends;
    std::vector<FriendRequest> pendingRequests;  // Requests we've received
    std::vector<FriendRequest> sentRequests;     // Requests we've sent

    // Callbacks
    std::function<void(const FriendRequest&)> onFriendRequestReceived;
    std::function<void(const Friend&)> onFriendAdded;
    std::function<void(uint64_t)> onFriendRemoved;
    std::function<void(const Friend&)> onFriendStatusChanged;
    std::function<void(uint64_t, const std::string&)> onPartyInviteReceived;
};
