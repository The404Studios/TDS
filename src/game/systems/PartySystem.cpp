#include "PartySystem.h"
#include <algorithm>
#include <iostream>
#include <chrono>

// ========== Party Implementation ==========

Party::Party(uint64_t partyId, uint64_t leaderAccountId)
    : partyId(partyId), leaderId(leaderAccountId) {
}

void Party::addMember(const PartyMember& member) {
    if (!hasMember(member.accountId)) {
        members.push_back(member);
        std::cout << "[Party] Added member: " << member.username << " (total: " << members.size() << ")" << std::endl;
    }
}

void Party::removeMember(uint64_t accountId) {
    auto it = std::remove_if(members.begin(), members.end(),
        [accountId](const PartyMember& m) { return m.accountId == accountId; });

    if (it != members.end()) {
        std::cout << "[Party] Removed member: " << it->username << std::endl;
        members.erase(it, members.end());
    }
}

bool Party::hasMember(uint64_t accountId) const {
    return std::any_of(members.begin(), members.end(),
        [accountId](const PartyMember& m) { return m.accountId == accountId; });
}

PartyMember* Party::getMember(uint64_t accountId) {
    auto it = std::find_if(members.begin(), members.end(),
        [accountId](const PartyMember& m) { return m.accountId == accountId; });

    if (it != members.end()) {
        return &(*it);
    }
    return nullptr;
}

void Party::setLeader(uint64_t accountId) {
    // Remove leader status from current leader
    if (PartyMember* currentLeader = getMember(leaderId)) {
        currentLeader->isLeader = false;
    }

    // Set new leader
    leaderId = accountId;
    if (PartyMember* newLeader = getMember(accountId)) {
        newLeader->isLeader = true;
        std::cout << "[Party] New leader: " << newLeader->username << std::endl;
    }
}

void Party::setMemberReady(uint64_t accountId, bool ready) {
    if (PartyMember* member = getMember(accountId)) {
        member->isReady = ready;
        std::cout << "[Party] " << member->username << " is " << (ready ? "ready" : "not ready") << std::endl;
    }
}

bool Party::areAllMembersReady() const {
    if (members.empty()) return false;

    return std::all_of(members.begin(), members.end(),
        [](const PartyMember& m) { return m.isReady; });
}

int Party::getReadyCount() const {
    return static_cast<int>(std::count_if(members.begin(), members.end(),
        [](const PartyMember& m) { return m.isReady; }));
}

// ========== PartySystem Implementation ==========

PartySystem::PartySystem()
    : inMatchmaking(false), localAccountId(0) {
}

PartySystem::~PartySystem() {
}

void PartySystem::createParty(uint64_t accountId, const std::string& username) {
    if (currentParty) {
        std::cout << "[PartySystem] Already in a party" << std::endl;
        return;
    }

    localAccountId = accountId;

    // Generate party ID (in production, this would come from server)
    uint64_t partyId = std::chrono::system_clock::now().time_since_epoch().count();

    currentParty = std::make_unique<Party>(partyId, accountId);

    // Add self as first member
    PartyMember selfMember;
    selfMember.accountId = accountId;
    selfMember.username = username;
    selfMember.isLeader = true;
    selfMember.isReady = false;

    currentParty->addMember(selfMember);

    std::cout << "[PartySystem] Created party " << partyId << " with leader " << username << std::endl;

    if (onPartyCreated) {
        onPartyCreated(currentParty.get());
    }

    // TODO: Send network packet to server
}

void PartySystem::leaveParty() {
    if (!currentParty) {
        return;
    }

    bool wasLeader = currentParty->isLeader(localAccountId);

    if (wasLeader) {
        // If leader leaves, disband party
        disbandParty();
    } else {
        // Just leave
        currentParty->removeMember(localAccountId);
        currentParty.reset();

        std::cout << "[PartySystem] Left party" << std::endl;

        if (onPartyDisbanded) {
            onPartyDisbanded();
        }
    }

    // TODO: Send network packet to server
}

void PartySystem::disbandParty() {
    if (!currentParty) {
        return;
    }

    std::cout << "[PartySystem] Disbanded party" << std::endl;

    currentParty.reset();

    if (onPartyDisbanded) {
        onPartyDisbanded();
    }

    // TODO: Send network packet to server
}

void PartySystem::inviteToParty(uint64_t accountId, const std::string& username) {
    if (!currentParty) {
        std::cout << "[PartySystem] Not in a party" << std::endl;
        return;
    }

    if (!currentParty->isLeader(localAccountId)) {
        std::cout << "[PartySystem] Only party leader can invite" << std::endl;
        return;
    }

    std::cout << "[PartySystem] Inviting " << username << " to party" << std::endl;

    // TODO: Send network packet to server to send invite
}

void PartySystem::acceptInvite(uint64_t partyId) {
    auto it = std::find_if(pendingInvites.begin(), pendingInvites.end(),
        [partyId](const PartyInvite& inv) { return inv.partyId == partyId; });

    if (it == pendingInvites.end()) {
        std::cout << "[PartySystem] Invite not found" << std::endl;
        return;
    }

    // Leave current party if in one
    if (currentParty) {
        leaveParty();
    }

    std::cout << "[PartySystem] Accepted invite to party " << partyId << std::endl;

    // Remove invite
    pendingInvites.erase(it);

    // TODO: Send network packet to server to join party
    // Server will send back party info and we'll create the party object
}

void PartySystem::declineInvite(uint64_t partyId) {
    auto it = std::remove_if(pendingInvites.begin(), pendingInvites.end(),
        [partyId](const PartyInvite& inv) { return inv.partyId == partyId; });

    if (it != pendingInvites.end()) {
        std::cout << "[PartySystem] Declined invite to party " << partyId << std::endl;
        pendingInvites.erase(it, pendingInvites.end());

        // TODO: Send network packet to server
    }
}

void PartySystem::kickMember(uint64_t accountId) {
    if (!currentParty) {
        return;
    }

    if (!currentParty->isLeader(localAccountId)) {
        std::cout << "[PartySystem] Only party leader can kick members" << std::endl;
        return;
    }

    if (PartyMember* member = currentParty->getMember(accountId)) {
        std::cout << "[PartySystem] Kicking " << member->username << " from party" << std::endl;
        currentParty->removeMember(accountId);

        if (onMemberLeft) {
            onMemberLeft(accountId);
        }

        // TODO: Send network packet to server
    }
}

void PartySystem::promoteToLeader(uint64_t accountId) {
    if (!currentParty) {
        return;
    }

    if (!currentParty->isLeader(localAccountId)) {
        std::cout << "[PartySystem] Only party leader can promote members" << std::endl;
        return;
    }

    currentParty->setLeader(accountId);

    // TODO: Send network packet to server
}

void PartySystem::setReady(bool ready) {
    if (!currentParty) {
        return;
    }

    currentParty->setMemberReady(localAccountId, ready);

    // Check if all members ready
    if (ready && currentParty->areAllMembersReady()) {
        std::cout << "[PartySystem] All members ready!" << std::endl;
    }

    // TODO: Send network packet to server
}

void PartySystem::startMatchmaking() {
    if (!currentParty) {
        std::cout << "[PartySystem] Not in a party" << std::endl;
        return;
    }

    if (!currentParty->isLeader(localAccountId)) {
        std::cout << "[PartySystem] Only party leader can start matchmaking" << std::endl;
        return;
    }

    if (!currentParty->areAllMembersReady()) {
        int ready = currentParty->getReadyCount();
        int total = currentParty->getMemberCount();
        std::cout << "[PartySystem] Not all members ready (" << ready << "/" << total << ")" << std::endl;
        return;
    }

    inMatchmaking = true;
    std::cout << "[PartySystem] Started matchmaking with party of " << currentParty->getMemberCount() << std::endl;

    // TODO: Send network packet to server to join matchmaking queue
}

void PartySystem::stopMatchmaking() {
    if (!inMatchmaking) {
        return;
    }

    inMatchmaking = false;
    std::cout << "[PartySystem] Stopped matchmaking" << std::endl;

    // TODO: Send network packet to server
}
