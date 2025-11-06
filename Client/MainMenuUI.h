#pragma once
#include "UIManager.h"
#include "NetworkClient.h"
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"

// Main Menu UI - Access to stash, merchants, and lobby
class MainMenuUI : public BaseUI {
public:
    MainMenuUI(NetworkClient* netClient, uint64_t accId)
        : networkClient(netClient), accountId(accId), selectedOption(0) {}

    void update(float deltaTime) override;
    void render() override;
    void handleInput(char key) override;

    void setPlayerStats(const PlayerStats& stats) {
        playerStats = stats;
    }

private:
    NetworkClient* networkClient;
    uint64_t accountId;
    int selectedOption;
    PlayerStats playerStats;

    void selectOption();
    void handlePlayerDataResponse(const std::vector<uint8_t>& payload);
};
