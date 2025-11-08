#pragma once

#include "Scene.h"
#include "UIPanel.h"
#include "UIButton.h"
#include "UIText.h"
#include "UIInputField.h"
#include "../network/NetworkClient.h"
#include "../../common/DataStructures.h"
#include <memory>
#include <vector>
#include <functional>

// Lobby player entry - represents a player in the lobby
class LobbyPlayerEntry : public UIPanel {
public:
    LobbyPlayerEntry(const std::string& name, uint64_t accountId, bool isOwner, bool isReady);

    void setPlayerData(const std::string& name, bool isOwner, bool isReady);
    void setReady(bool ready);
    bool isReady() const { return ready; }
    uint64_t getAccountId() const { return accountId; }

    void update(float deltaTime) override;
    void render() override;

private:
    uint64_t accountId;
    bool isOwner;
    bool ready;

    std::shared_ptr<UIText> nameText;
    std::shared_ptr<UIText> statusText;
    std::shared_ptr<UIPanel> readyIndicator;
    std::shared_ptr<UIPanel> ownerBadge;
};

// Lobby Scene - Multiplayer lobby for creating/joining games
class LobbyScene : public Scene {
public:
    LobbyScene(NetworkClient* netClient, uint64_t accountId);

    void onLoad() override;
    void onUnload() override;
    void update(float deltaTime) override;
    void render() override;

    // Input handling
    void handleInput(char key);
    void handleMouseClick(float x, float y);
    void handleMouseMove(float x, float y);

    // Lobby management
    void createLobby(const std::string& lobbyName);
    void joinLobby(uint32_t lobbyId);
    void leaveLobby();
    void toggleReady();
    void startMatch();
    void kickPlayer(uint64_t targetAccountId);

    // Navigation callbacks
    void setOnLeaveLobby(std::function<void()> callback) { onLeaveLobbyCallback = callback; }
    void setOnMatchStart(std::function<void()> callback) { onMatchStartCallback = callback; }

private:
    void setupUI();
    void updateLobbyList();
    void updatePlayerList();
    void updateLobbyInfo();
    void processNetworkPackets();
    void updateHoverStates(float mouseX, float mouseY);

    // UI helpers
    void showLobbyList();
    void showLobbyRoom();
    void addChatMessage(const std::string& message);

    NetworkClient* networkClient;
    uint64_t accountId;
    float animTime;
    float lastMouseX, lastMouseY;

    // State
    enum class LobbyState {
        LOBBY_LIST,      // Browsing available lobbies
        LOBBY_ROOM,      // Inside a lobby
        CREATING_LOBBY   // Creating new lobby
    };
    LobbyState currentState;
    uint32_t currentLobbyId;
    bool isLobbyOwner;
    bool isReady;

    // Lobby data
    std::vector<Lobby> availableLobbies;
    Lobby* currentLobby;
    std::vector<LobbyMember> lobbyMembers;

    // Navigation callbacks
    std::function<void()> onLeaveLobbyCallback;
    std::function<void()> onMatchStartCallback;

    // UI Elements - Lobby List View
    std::shared_ptr<UIPanel> lobbyListPanel;
    std::shared_ptr<UIButton> createLobbyButton;
    std::shared_ptr<UIButton> refreshButton;
    std::shared_ptr<UIButton> backButton;
    std::shared_ptr<UIText> lobbyCountText;
    std::vector<std::shared_ptr<UIButton>> lobbyButtons;

    // UI Elements - Lobby Room View
    std::shared_ptr<UIPanel> lobbyRoomPanel;
    std::shared_ptr<UIText> lobbyNameText;
    std::shared_ptr<UIPanel> playersPanel;
    std::vector<std::shared_ptr<LobbyPlayerEntry>> playerEntries;
    std::shared_ptr<UIButton> readyButton;
    std::shared_ptr<UIButton> startMatchButton;
    std::shared_ptr<UIButton> leaveLobbyButton;

    // UI Elements - Chat
    std::shared_ptr<UIPanel> chatPanel;
    std::shared_ptr<UIInputField> chatInput;
    std::vector<std::shared_ptr<UIText>> chatMessages;
    std::shared_ptr<UIButton> sendChatButton;

    // UI Elements - Create Lobby
    std::shared_ptr<UIPanel> createLobbyPanel;
    std::shared_ptr<UIInputField> lobbyNameInput;
    std::shared_ptr<UIInputField> maxPlayersInput;
    std::shared_ptr<UIButton> confirmCreateButton;
    std::shared_ptr<UIButton> cancelCreateButton;

    // Title and info
    std::shared_ptr<UIText> titleText;
    std::shared_ptr<UIText> subtitleText;
    std::shared_ptr<UIPanel> infoPanel;
    std::shared_ptr<UIText> infoText;
};
