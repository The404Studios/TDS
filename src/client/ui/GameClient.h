#pragma once
#include "UIManager.h"
#include "../network/NetworkClient.h"
#include "../../common/NetworkProtocol.h"
#include "../../common/DataStructures.h"
#include <vector>
#include <cmath>

// In-game client (FPS gameplay)
class GameClient : public BaseUI {
public:
    GameClient(NetworkClient* netClient, uint64_t accId)
        : networkClient(netClient), accountId(accId),
          playerX(0), playerY(1.7f), playerZ(0), playerYaw(0), playerPitch(0),
          health(440.0f), alive(true), extracted(false) {}

    void update(float deltaTime) override;
    void render() override;
    void handleInput(char key) override;

private:
    NetworkClient* networkClient;
    uint64_t accountId;

    // Player state
    float playerX, playerY, playerZ;
    float playerYaw, playerPitch;
    float health;
    bool alive;
    bool extracted;

    void renderGround();
    void renderHUD();
    void renderDeathScreen();
    void renderExtractedScreen();
    void sendPositionUpdate();
    void handleSpawnInfo(const std::vector<uint8_t>& payload);
    void handlePlayerDamage(const std::vector<uint8_t>& payload);
    void handlePlayerDeath(const std::vector<uint8_t>& payload);
    void handleExtractionComplete(const std::vector<uint8_t>& payload);
    bool isNearExtraction();
    void requestExtraction();
};
