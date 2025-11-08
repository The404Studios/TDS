#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>

/**
 * PartyMember - Represents a member of the party
 */
struct PartyMember {
    uint64_t accountId;
    std::string username;
    int level;
    bool isReady;
    bool isLeader;

    PartyMember() : accountId(0), level(1), isReady(false), isLeader(false) {}
};

/**
 * PartyInvite - Pending party invitation
 */
struct PartyInvite {
    uint64_t partyId;
    uint64_t fromAccountId;
    std::string fromUsername;
    uint64_t timestamp;

    PartyInvite() : partyId(0), fromAccountId(0), timestamp(0) {}
};

/**
 * Party - Represents a group of players
 */
class Party {
public:
    Party(uint64_t partyId, uint64_t leaderAccountId);
    ~Party() = default;

    // Member management
    void addMember(const PartyMember& member);
    void removeMember(uint64_t accountId);
    bool hasMember(uint64_t accountId) const;
    PartyMember* getMember(uint64_t accountId);
    const std::vector<PartyMember>& getMembers() const { return members; }
    int getMemberCount() const { return static_cast<int>(members.size()); }

    // Leader management
    void setLeader(uint64_t accountId);
    uint64_t getLeaderId() const { return leaderId; }
    bool isLeader(uint64_t accountId) const { return accountId == leaderId; }

    // Ready status
    void setMemberReady(uint64_t accountId, bool ready);
    bool areAllMembersReady() const;
    int getReadyCount() const;

    // Getters
    uint64_t getPartyId() const { return partyId; }

private:
    uint64_t partyId;
    uint64_t leaderId;
    std::vector<PartyMember> members;
};

/**
 * PartySystem - Manages party formation and invites
 */
class PartySystem {
public:
    PartySystem();
    ~PartySystem();

    // Party creation/management
    void createParty(uint64_t accountId, const std::string& username);
    void leaveParty();
    void disbandParty();
    bool isInParty() const { return currentParty != nullptr; }
    Party* getCurrentParty() { return currentParty.get(); }

    // Invites
    void inviteToParty(uint64_t accountId, const std::string& username);
    void acceptInvite(uint64_t partyId);
    void declineInvite(uint64_t partyId);
    const std::vector<PartyInvite>& getPendingInvites() const { return pendingInvites; }

    // Member actions
    void kickMember(uint64_t accountId);
    void promoteToLeader(uint64_t accountId);
    void setReady(bool ready);

    // Matchmaking
    void startMatchmaking();
    void stopMatchmaking();
    bool isMatchmaking() const { return inMatchmaking; }

    // Callbacks
    void setOnPartyCreated(std::function<void(Party*)> callback) {
        onPartyCreated = callback;
    }
    void setOnPartyDisbanded(std::function<void()> callback) {
        onPartyDisbanded = callback;
    }
    void setOnMemberJoined(std::function<void(const PartyMember&)> callback) {
        onMemberJoined = callback;
    }
    void setOnMemberLeft(std::function<void(uint64_t)> callback) {
        onMemberLeft = callback;
    }
    void setOnInviteReceived(std::function<void(const PartyInvite&)> callback) {
        onInviteReceived = callback;
    }
    void setOnMatchFound(std::function<void()> callback) {
        onMatchFound = callback;
    }

private:
    std::unique_ptr<Party> currentParty;
    std::vector<PartyInvite> pendingInvites;
    bool inMatchmaking;

    uint64_t localAccountId;  // Local player's account ID

    // Callbacks
    std::function<void(Party*)> onPartyCreated;
    std::function<void()> onPartyDisbanded;
    std::function<void(const PartyMember&)> onMemberJoined;
    std::function<void(uint64_t)> onMemberLeft;
    std::function<void(const PartyInvite&)> onInviteReceived;
    std::function<void()> onMatchFound;
};
