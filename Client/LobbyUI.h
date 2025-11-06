#pragma once
#include "UIManager.h"
#include "NetworkClient.h"
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include <vector>
#include <cstring>

// Lobby UI
class LobbyUI : public BaseUI {
public:
    LobbyUI(NetworkClient* netClient, uint64_t accId)
        : networkClient(netClient), accountId(accId), inLobby(false),
          isReady(false), isOwner(false), inQueue(false), selectedAction(0) {}

    void update(float deltaTime) override;
    void render() override;
    void handleInput(char key) override;
    void handleMouseClick(float x, float y) override;

private:
    NetworkClient* networkClient;
    uint64_t accountId;
    uint64_t currentLobbyId;
    bool inLobby;
    bool isReady;
    bool isOwner;
    bool inQueue;
    int selectedAction;
    std::vector<LobbyMember> lobbyMembers;
    std::string statusMessage;

    void renderLobbySelection();
    void renderLobbyView();
    void createLobby();
    void toggleReady();
    void leaveLobby();
    void startQueue();
    void handleLobbyCreateResponse(const std::vector<uint8_t>& payload);
    void handleLobbyJoinResponse(const std::vector<uint8_t>& payload);
    void handleLobbyUpdate(const std::vector<uint8_t>& payload);
    void handleMatchFound(const std::vector<uint8_t>& payload);
};
