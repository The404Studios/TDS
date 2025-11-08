#include "FriendsSystem.h"
#include <algorithm>
#include <iostream>
#include <chrono>

FriendsSystem::FriendsSystem() {
}

FriendsSystem::~FriendsSystem() {
}

void FriendsSystem::addFriend(uint64_t accountId, const std::string& username) {
    // Check if already friends
    if (isFriend(accountId)) {
        std::cout << "[FriendsSystem] Already friends with " << username << std::endl;
        return;
    }

    Friend newFriend;
    newFriend.accountId = accountId;
    newFriend.username = username;
    newFriend.status = Friend::Status::OFFLINE;

    friends.push_back(newFriend);

    std::cout << "[FriendsSystem] Added friend: " << username << std::endl;

    if (onFriendAdded) {
        onFriendAdded(newFriend);
    }
}

void FriendsSystem::removeFriend(uint64_t accountId) {
    auto it = std::remove_if(friends.begin(), friends.end(),
        [accountId](const Friend& f) { return f.accountId == accountId; });

    if (it != friends.end()) {
        std::cout << "[FriendsSystem] Removed friend: " << it->username << std::endl;
        friends.erase(it, friends.end());

        if (onFriendRemoved) {
            onFriendRemoved(accountId);
        }
    }
}

bool FriendsSystem::isFriend(uint64_t accountId) const {
    return std::any_of(friends.begin(), friends.end(),
        [accountId](const Friend& f) { return f.accountId == accountId; });
}

Friend* FriendsSystem::getFriend(uint64_t accountId) {
    auto it = std::find_if(friends.begin(), friends.end(),
        [accountId](const Friend& f) { return f.accountId == accountId; });

    if (it != friends.end()) {
        return &(*it);
    }
    return nullptr;
}

void FriendsSystem::sendFriendRequest(uint64_t toAccountId, const std::string& toUsername) {
    // Check if already friends
    if (isFriend(toAccountId)) {
        std::cout << "[FriendsSystem] Already friends with user " << toAccountId << std::endl;
        return;
    }

    // Check if request already sent
    auto it = std::find_if(sentRequests.begin(), sentRequests.end(),
        [toAccountId](const FriendRequest& req) { return req.toAccountId == toAccountId; });

    if (it != sentRequests.end()) {
        std::cout << "[FriendsSystem] Friend request already sent to " << toUsername << std::endl;
        return;
    }

    FriendRequest request;
    request.toAccountId = toAccountId;
    request.toUsername = toUsername;
    request.timestamp = std::chrono::system_clock::now().time_since_epoch().count();

    sentRequests.push_back(request);

    std::cout << "[FriendsSystem] Sent friend request to " << toUsername << std::endl;

    // TODO: Send network packet to server
}

void FriendsSystem::acceptFriendRequest(uint64_t fromAccountId) {
    auto it = std::find_if(pendingRequests.begin(), pendingRequests.end(),
        [fromAccountId](const FriendRequest& req) { return req.fromAccountId == fromAccountId; });

    if (it != pendingRequests.end()) {
        std::cout << "[FriendsSystem] Accepted friend request from " << it->fromUsername << std::endl;

        // Add to friends list
        addFriend(it->fromAccountId, it->fromUsername);

        // Remove from pending
        pendingRequests.erase(it);

        // TODO: Send network packet to server
    }
}

void FriendsSystem::declineFriendRequest(uint64_t fromAccountId) {
    auto it = std::remove_if(pendingRequests.begin(), pendingRequests.end(),
        [fromAccountId](const FriendRequest& req) { return req.fromAccountId == fromAccountId; });

    if (it != pendingRequests.end()) {
        std::cout << "[FriendsSystem] Declined friend request from " << it->fromUsername << std::endl;
        pendingRequests.erase(it, pendingRequests.end());

        // TODO: Send network packet to server
    }
}

void FriendsSystem::updateFriendStatus(uint64_t accountId, Friend::Status status) {
    Friend* friendPtr = getFriend(accountId);
    if (friendPtr) {
        friendPtr->status = status;

        if (onFriendStatusChanged) {
            onFriendStatusChanged(*friendPtr);
        }
    }
}

void FriendsSystem::updateFriendParty(uint64_t accountId, uint64_t partyId, bool isLeader) {
    Friend* friendPtr = getFriend(accountId);
    if (friendPtr) {
        friendPtr->partyId = partyId;
        friendPtr->isPartyLeader = isLeader;

        if (onFriendStatusChanged) {
            onFriendStatusChanged(*friendPtr);
        }
    }
}

std::vector<Friend*> FriendsSystem::getOnlineFriends() {
    std::vector<Friend*> online;
    for (auto& friendEntry : friends) {
        if (friendEntry.status != Friend::Status::OFFLINE) {
            online.push_back(&friendEntry);
        }
    }
    return online;
}

std::vector<Friend*> FriendsSystem::getFriendsInMenu() {
    std::vector<Friend*> inMenu;
    for (auto& friendEntry : friends) {
        if (friendEntry.status == Friend::Status::IN_MENU) {
            inMenu.push_back(&friendEntry);
        }
    }
    return inMenu;
}

std::vector<Friend*> FriendsSystem::getFriendsInLobby() {
    std::vector<Friend*> inLobby;
    for (auto& friendEntry : friends) {
        if (friendEntry.status == Friend::Status::IN_LOBBY) {
            inLobby.push_back(&friendEntry);
        }
    }
    return inLobby;
}

void FriendsSystem::inviteToParty(uint64_t friendAccountId) {
    Friend* friendPtr = getFriend(friendAccountId);
    if (!friendPtr) {
        std::cout << "[FriendsSystem] Cannot invite - not in friends list" << std::endl;
        return;
    }

    std::cout << "[FriendsSystem] Inviting " << friendPtr->username << " to party" << std::endl;

    // TODO: Send network packet to server to invite friend to party
}
