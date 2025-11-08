#pragma once

#include "../../engine/scene/Scene.h"
#include "../../client/ui/GameObject.h"
#include "../../client/ui/UIButton.h"
#include "../../client/ui/Text.h"
#include "../systems/FriendsSystem.h"
#include "../systems/PartySystem.h"
#include <memory>
#include <vector>

/**
 * LobbyScene - Party formation and friends management
 * Players can form parties, invite friends, and queue for 100-player matches
 */
class LobbyScene : public Scene {
public:
    LobbyScene();
    ~LobbyScene() override = default;

    // Scene lifecycle
    void onEnter() override;
    void onExit() override;
    void onPause() override;
    void onResume() override;

    // Update
    void tick(float dt) override;
    void fixedTick(float fixedDt) override;

    // Input
    void handleInput(const InputState& input) override;

    // Rendering
    void render() override;

private:
    // Systems
    std::unique_ptr<FriendsSystem> friendsSystem;
    std::unique_ptr<PartySystem> partySystem;

    // UI State
    enum class Tab {
        PARTY,
        FRIENDS,
        FRIEND_REQUESTS
    };
    Tab currentTab;

    // UI Elements
    std::shared_ptr<Text> titleText;
    std::shared_ptr<Text> statusText;

    // Tab buttons
    std::shared_ptr<UIButton> partyTabButton;
    std::shared_ptr<UIButton> friendsTabButton;
    std::shared_ptr<UIButton> requestsTabButton;

    // Main action buttons
    std::shared_ptr<UIButton> createPartyButton;
    std::shared_ptr<UIButton> leavePartyButton;
    std::shared_ptr<UIButton> readyButton;
    std::shared_ptr<UIButton> startMatchmakingButton;
    std::shared_ptr<UIButton> backButton;

    // Party member list
    struct PartyMemberUI {
        PartyMember* member;
        std::shared_ptr<Text> nameText;
        std::shared_ptr<Text> statusText;
        std::shared_ptr<UIButton> kickButton;
        std::shared_ptr<UIButton> promoteButton;
    };
    std::vector<PartyMemberUI> partyMemberUI;

    // Friends list
    struct FriendUI {
        Friend* friendPtr;
        std::shared_ptr<Text> nameText;
        std::shared_ptr<Text> statusText;
        std::shared_ptr<UIButton> inviteButton;
        std::shared_ptr<UIButton> removeButton;
    };
    std::vector<FriendUI> friendsUI;

    // Friend request list
    struct FriendRequestUI {
        FriendRequest* request;
        std::shared_ptr<Text> nameText;
        std::shared_ptr<UIButton> acceptButton;
        std::shared_ptr<UIButton> declineButton;
    };
    std::vector<FriendRequestUI> requestsUI;

    // Add friend
    std::shared_ptr<UIButton> addFriendButton;

    // Matchmaking state
    bool isMatchmaking;
    float matchmakingTime;
    int partySize;

    // Methods
    void initializeUI();
    void initializeSystems();
    void updatePartyUI();
    void updateFriendsUI();
    void updateRequestsUI();
    void switchTab(Tab tab);

    // Party callbacks
    void onCreatePartyClicked();
    void onLeavePartyClicked();
    void onReadyClicked();
    void onStartMatchmakingClicked();
    void onKickMember(uint64_t accountId);
    void onPromoteMember(uint64_t accountId);

    // Friends callbacks
    void onInviteFriend(uint64_t accountId);
    void onRemoveFriend(uint64_t accountId);
    void onAddFriendClicked();

    // Request callbacks
    void onAcceptRequest(uint64_t fromAccountId);
    void onDeclineRequest(uint64_t fromAccountId);

    // System callbacks
    void onPartyCreated(Party* party);
    void onPartyDisbanded();
    void onMemberJoined(const PartyMember& member);
    void onMemberLeft(uint64_t accountId);
    void onMatchFound();

    // Utility
    void onBackClicked();
};
