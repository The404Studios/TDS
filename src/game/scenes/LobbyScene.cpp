#include "LobbyScene.h"
#include "../../engine/GameEngine.h"
#include <sstream>
#include <iostream>

LobbyScene::LobbyScene()
    : currentTab(Tab::PARTY),
      isMatchmaking(false),
      matchmakingTime(0.0f),
      partySize(1) {
}

void LobbyScene::onEnter() {
    std::cout << "[LobbyScene] Entering lobby" << std::endl;

    initializeSystems();
    initializeUI();
    switchTab(Tab::PARTY);
}

void LobbyScene::onExit() {
    std::cout << "[LobbyScene] Exiting lobby" << std::endl;

    // Stop matchmaking if active
    if (isMatchmaking && partySystem) {
        partySystem->stopMatchmaking();
    }
}

void LobbyScene::onPause() {
}

void LobbyScene::onResume() {
    updatePartyUI();
    updateFriendsUI();
    updateRequestsUI();
}

void LobbyScene::tick(float dt) {
    if (isMatchmaking) {
        matchmakingTime += dt;

        // Update status text
        std::stringstream ss;
        ss << "Searching for 100-player match";
        int dots = (int)(matchmakingTime * 2.0f) % 4;
        for (int i = 0; i < dots; i++) {
            ss << ".";
        }
        ss << " (" << partySize << " in party)";
        statusText->setText(ss.str());
    }
}

void LobbyScene::fixedTick(float fixedDt) {
}

void LobbyScene::handleInput(const InputState& input) {
    // Handle tab switching
    if (input.keys['1']) {
        switchTab(Tab::PARTY);
    }
    if (input.keys['2']) {
        switchTab(Tab::FRIENDS);
    }
    if (input.keys['3']) {
        switchTab(Tab::FRIEND_REQUESTS);
    }

    // ESC to go back
    if (input.keys[27]) {
        onBackClicked();
    }
}

void LobbyScene::render() {
    // Render would happen here in a real implementation
}

void LobbyScene::initializeUI() {
    // Title
    titleText = std::make_shared<Text>("LobbyTitle", "LOBBY");
    titleText->setPosition(960, 50);
    titleText->setSize(48);
    titleText->setColor(Color(255, 255, 255));

    // Status text
    statusText = std::make_shared<Text>("StatusText", "");
    statusText->setPosition(960, 100);
    statusText->setSize(20);
    statusText->setColor(Color(200, 200, 200));

    // Tab buttons
    partyTabButton = std::make_shared<UIButton>("PartyTab", "PARTY");
    partyTabButton->setPosition(200, 150);
    partyTabButton->setSize(150, 50);
    partyTabButton->setOnClick([this]() { switchTab(Tab::PARTY); });

    friendsTabButton = std::make_shared<UIButton>("FriendsTab", "FRIENDS");
    friendsTabButton->setPosition(400, 150);
    friendsTabButton->setSize(150, 50);
    friendsTabButton->setOnClick([this]() { switchTab(Tab::FRIENDS); });

    requestsTabButton = std::make_shared<UIButton>("RequestsTab", "REQUESTS");
    requestsTabButton->setPosition(600, 150);
    requestsTabButton->setSize(150, 50);
    requestsTabButton->setOnClick([this]() { switchTab(Tab::FRIEND_REQUESTS); });

    // Party action buttons
    createPartyButton = std::make_shared<UIButton>("CreateParty", "CREATE PARTY");
    createPartyButton->setPosition(200, 250);
    createPartyButton->setSize(200, 60);
    createPartyButton->setOnClick([this]() { onCreatePartyClicked(); });

    leavePartyButton = std::make_shared<UIButton>("LeaveParty", "LEAVE PARTY");
    leavePartyButton->setPosition(450, 250);
    leavePartyButton->setSize(200, 60);
    leavePartyButton->setOnClick([this]() { onLeavePartyClicked(); });

    readyButton = std::make_shared<UIButton>("Ready", "READY");
    readyButton->setPosition(700, 250);
    readyButton->setSize(200, 60);
    readyButton->setOnClick([this]() { onReadyClicked(); });

    startMatchmakingButton = std::make_shared<UIButton>("StartMatchmaking", "START MATCHMAKING");
    startMatchmakingButton->setPosition(950, 250);
    startMatchmakingButton->setSize(300, 60);
    startMatchmakingButton->setOnClick([this]() { onStartMatchmakingClicked(); });

    // Add friend button
    addFriendButton = std::make_shared<UIButton>("AddFriend", "ADD FRIEND");
    addFriendButton->setPosition(200, 250);
    addFriendButton->setSize(200, 60);
    addFriendButton->setOnClick([this]() { onAddFriendClicked(); });

    // Back button
    backButton = std::make_shared<UIButton>("Back", "BACK TO MENU");
    backButton->setPosition(100, 950);
    backButton->setSize(250, 60);
    backButton->setOnClick([this]() { onBackClicked(); });
}

void LobbyScene::initializeSystems() {
    // Initialize friends system
    friendsSystem = std::make_unique<FriendsSystem>();

    // Add some test friends
    friendsSystem->addFriend(1001, "Player1");
    friendsSystem->addFriend(1002, "Player2");
    friendsSystem->addFriend(1003, "Player3");

    // Update their statuses
    friendsSystem->updateFriendStatus(1001, Friend::Status::ONLINE);
    friendsSystem->updateFriendStatus(1002, Friend::Status::IN_MENU);
    friendsSystem->updateFriendStatus(1003, Friend::Status::OFFLINE);

    // Initialize party system
    partySystem = std::make_unique<PartySystem>();

    // Setup callbacks
    partySystem->setOnPartyCreated([this](Party* party) { onPartyCreated(party); });
    partySystem->setOnPartyDisbanded([this]() { onPartyDisbanded(); });
    partySystem->setOnMemberJoined([this](const PartyMember& member) { onMemberJoined(member); });
    partySystem->setOnMemberLeft([this](uint64_t accountId) { onMemberLeft(accountId); });
    partySystem->setOnMatchFound([this]() { onMatchFound(); });
}

void LobbyScene::updatePartyUI() {
    // Clear existing UI
    partyMemberUI.clear();

    if (!partySystem || !partySystem->isInParty()) {
        statusText->setText("Not in a party");
        return;
    }

    Party* party = partySystem->getCurrentParty();
    if (!party) return;

    // Create UI for each member
    int yOffset = 350;
    for (const auto& member : party->getMembers()) {
        PartyMemberUI memberUI;
        memberUI.member = const_cast<PartyMember*>(&member);

        // Name
        std::stringstream nameStr;
        nameStr << member.username;
        if (member.isLeader) nameStr << " (Leader)";

        memberUI.nameText = std::make_shared<Text>("Member_" + std::to_string(member.accountId), nameStr.str());
        memberUI.nameText->setPosition(250, yOffset);
        memberUI.nameText->setSize(24);
        memberUI.nameText->setColor(member.isLeader ? Color(255, 215, 0) : Color(255, 255, 255));

        // Status
        std::string statusStr = member.isReady ? "READY" : "Not Ready";
        memberUI.statusText = std::make_shared<Text>("Status_" + std::to_string(member.accountId), statusStr);
        memberUI.statusText->setPosition(450, yOffset);
        memberUI.statusText->setSize(20);
        memberUI.statusText->setColor(member.isReady ? Color(0, 255, 0) : Color(200, 200, 200));

        // Kick button (only show for leader, not for self)
        if (party->isLeader(0) && member.accountId != 0 && !member.isLeader) {  // TODO: Use actual local account ID
            memberUI.kickButton = std::make_shared<UIButton>("Kick_" + std::to_string(member.accountId), "KICK");
            memberUI.kickButton->setPosition(650, yOffset);
            memberUI.kickButton->setSize(100, 40);
            memberUI.kickButton->setOnClick([this, accountId = member.accountId]() { onKickMember(accountId); });
        }

        // Promote button (only for leader)
        if (party->isLeader(0) && member.accountId != 0 && !member.isLeader) {  // TODO: Use actual local account ID
            memberUI.promoteButton = std::make_shared<UIButton>("Promote_" + std::to_string(member.accountId), "PROMOTE");
            memberUI.promoteButton->setPosition(800, yOffset);
            memberUI.promoteButton->setSize(120, 40);
            memberUI.promoteButton->setOnClick([this, accountId = member.accountId]() { onPromoteMember(accountId); });
        }

        partyMemberUI.push_back(memberUI);
        yOffset += 60;
    }

    partySize = party->getMemberCount();

    // Update status
    std::stringstream ss;
    ss << "Party Size: " << party->getMemberCount();
    ss << " | Ready: " << party->getReadyCount() << "/" << party->getMemberCount();
    statusText->setText(ss.str());
}

void LobbyScene::updateFriendsUI() {
    // Clear existing UI
    friendsUI.clear();

    if (!friendsSystem) return;

    const auto& friends = friendsSystem->getFriendsList();

    int yOffset = 350;
    for (const auto& friendEntry : friends) {
        FriendUI friendUI;
        friendUI.friendPtr = const_cast<Friend*>(&friendEntry);

        // Name
        friendUI.nameText = std::make_shared<Text>("Friend_" + std::to_string(friendEntry.accountId), friendEntry.username);
        friendUI.nameText->setPosition(250, yOffset);
        friendUI.nameText->setSize(24);
        friendUI.nameText->setColor(Color(255, 255, 255));

        // Status
        std::string statusStr;
        Color statusColor;
        switch (friendEntry.status) {
            case Friend::Status::ONLINE:
                statusStr = "Online";
                statusColor = Color(0, 255, 0);
                break;
            case Friend::Status::IN_MENU:
                statusStr = "In Menu";
                statusColor = Color(100, 255, 100);
                break;
            case Friend::Status::IN_LOBBY:
                statusStr = "In Lobby";
                statusColor = Color(255, 255, 0);
                break;
            case Friend::Status::IN_RAID:
                statusStr = "In Raid";
                statusColor = Color(255, 100, 100);
                break;
            default:
                statusStr = "Offline";
                statusColor = Color(128, 128, 128);
                break;
        }

        friendUI.statusText = std::make_shared<Text>("Status_" + std::to_string(friendEntry.accountId), statusStr);
        friendUI.statusText->setPosition(450, yOffset);
        friendUI.statusText->setSize(20);
        friendUI.statusText->setColor(statusColor);

        // Invite button (only if online and we're party leader)
        if (friendEntry.status != Friend::Status::OFFLINE && partySystem && partySystem->isInParty()) {
            friendUI.inviteButton = std::make_shared<UIButton>("Invite_" + std::to_string(friendEntry.accountId), "INVITE");
            friendUI.inviteButton->setPosition(650, yOffset);
            friendUI.inviteButton->setSize(120, 40);
            friendUI.inviteButton->setOnClick([this, accountId = friendEntry.accountId]() { onInviteFriend(accountId); });
        }

        // Remove button
        friendUI.removeButton = std::make_shared<UIButton>("Remove_" + std::to_string(friendEntry.accountId), "REMOVE");
        friendUI.removeButton->setPosition(800, yOffset);
        friendUI.removeButton->setSize(120, 40);
        friendUI.removeButton->setOnClick([this, accountId = friendEntry.accountId]() { onRemoveFriend(accountId); });

        friendsUI.push_back(friendUI);
        yOffset += 60;
    }
}

void LobbyScene::updateRequestsUI() {
    // Clear existing UI
    requestsUI.clear();

    if (!friendsSystem) return;

    const auto& requests = friendsSystem->getPendingRequests();

    int yOffset = 350;
    for (const auto& request : requests) {
        FriendRequestUI requestUI;
        requestUI.request = const_cast<FriendRequest*>(&request);

        // Name
        requestUI.nameText = std::make_shared<Text>("Request_" + std::to_string(request.fromAccountId),
                                                     request.fromUsername + " wants to be friends");
        requestUI.nameText->setPosition(250, yOffset);
        requestUI.nameText->setSize(24);
        requestUI.nameText->setColor(Color(255, 255, 255));

        // Accept button
        requestUI.acceptButton = std::make_shared<UIButton>("Accept_" + std::to_string(request.fromAccountId), "ACCEPT");
        requestUI.acceptButton->setPosition(650, yOffset);
        requestUI.acceptButton->setSize(120, 40);
        requestUI.acceptButton->setOnClick([this, accountId = request.fromAccountId]() { onAcceptRequest(accountId); });

        // Decline button
        requestUI.declineButton = std::make_shared<UIButton>("Decline_" + std::to_string(request.fromAccountId), "DECLINE");
        requestUI.declineButton->setPosition(800, yOffset);
        requestUI.declineButton->setSize(120, 40);
        requestUI.declineButton->setOnClick([this, accountId = request.fromAccountId]() { onDeclineRequest(accountId); });

        requestsUI.push_back(requestUI);
        yOffset += 60;
    }
}

void LobbyScene::switchTab(Tab tab) {
    currentTab = tab;

    // Update UI based on tab
    switch (tab) {
        case Tab::PARTY:
            updatePartyUI();
            break;
        case Tab::FRIENDS:
            updateFriendsUI();
            break;
        case Tab::FRIEND_REQUESTS:
            updateRequestsUI();
            break;
    }
}

// Party callbacks

void LobbyScene::onCreatePartyClicked() {
    if (partySystem) {
        partySystem->createParty(0, "LocalPlayer");  // TODO: Use actual account ID and username
    }
}

void LobbyScene::onLeavePartyClicked() {
    if (partySystem) {
        partySystem->leaveParty();
    }
}

void LobbyScene::onReadyClicked() {
    if (partySystem && partySystem->isInParty()) {
        Party* party = partySystem->getCurrentParty();
        if (party) {
            PartyMember* member = party->getMember(0);  // TODO: Use actual account ID
            bool newReadyState = member ? !member->isReady : true;
            partySystem->setReady(newReadyState);

            readyButton->setLabel(newReadyState ? "NOT READY" : "READY");
            updatePartyUI();
        }
    }
}

void LobbyScene::onStartMatchmakingClicked() {
    if (isMatchmaking) {
        partySystem->stopMatchmaking();
        isMatchmaking = false;
        matchmakingTime = 0.0f;
        startMatchmakingButton->setLabel("START MATCHMAKING");
        statusText->setText("Matchmaking stopped");
    } else {
        partySystem->startMatchmaking();
        isMatchmaking = true;
        matchmakingTime = 0.0f;
        startMatchmakingButton->setLabel("STOP MATCHMAKING");
    }
}

void LobbyScene::onKickMember(uint64_t accountId) {
    if (partySystem) {
        partySystem->kickMember(accountId);
        updatePartyUI();
    }
}

void LobbyScene::onPromoteMember(uint64_t accountId) {
    if (partySystem) {
        partySystem->promoteToLeader(accountId);
        updatePartyUI();
    }
}

// Friends callbacks

void LobbyScene::onInviteFriend(uint64_t accountId) {
    if (friendsSystem && partySystem) {
        Friend* friendPtr = friendsSystem->getFriend(accountId);
        if (friendPtr) {
            partySystem->inviteToParty(accountId, friendPtr->username);
            std::cout << "[LobbyScene] Invited " << friendPtr->username << " to party" << std::endl;
        }
    }
}

void LobbyScene::onRemoveFriend(uint64_t accountId) {
    if (friendsSystem) {
        friendsSystem->removeFriend(accountId);
        updateFriendsUI();
    }
}

void LobbyScene::onAddFriendClicked() {
    std::cout << "[LobbyScene] Add friend clicked - would show input dialog" << std::endl;
    // TODO: Show input dialog to enter friend's username
}

// Request callbacks

void LobbyScene::onAcceptRequest(uint64_t fromAccountId) {
    if (friendsSystem) {
        friendsSystem->acceptFriendRequest(fromAccountId);
        updateRequestsUI();
        updateFriendsUI();
    }
}

void LobbyScene::onDeclineRequest(uint64_t fromAccountId) {
    if (friendsSystem) {
        friendsSystem->declineFriendRequest(fromAccountId);
        updateRequestsUI();
    }
}

// System callbacks

void LobbyScene::onPartyCreated(Party* party) {
    std::cout << "[LobbyScene] Party created" << std::endl;
    updatePartyUI();
}

void LobbyScene::onPartyDisbanded() {
    std::cout << "[LobbyScene] Party disbanded" << std::endl;
    isMatchmaking = false;
    updatePartyUI();
}

void LobbyScene::onMemberJoined(const PartyMember& member) {
    std::cout << "[LobbyScene] " << member.username << " joined party" << std::endl;
    updatePartyUI();
}

void LobbyScene::onMemberLeft(uint64_t accountId) {
    std::cout << "[LobbyScene] Member left party" << std::endl;
    updatePartyUI();
}

void LobbyScene::onMatchFound() {
    std::cout << "[LobbyScene] Match found! Loading into 100-player raid..." << std::endl;
    isMatchmaking = false;

    // Switch to raid scene
    SceneManager* sceneManager = ENGINE.getSceneManager();
    if (sceneManager) {
        sceneManager->switchTo("raid");
    }
}

void LobbyScene::onBackClicked() {
    std::cout << "[LobbyScene] Back button clicked" << std::endl;

    // Return to main menu
    SceneManager* sceneManager = ENGINE.getSceneManager();
    if (sceneManager) {
        sceneManager->switchTo("menu");
    }
}
