#pragma once

#include "Scene.h"
#include "UIPanel.h"
#include "UIButton.h"
#include "UIText.h"
#include "../network/NetworkClient.h"
#include <memory>
#include <functional>

// Main Menu Scene - Main hub with navigation buttons
class MainMenuScene : public Scene {
public:
    MainMenuScene(NetworkClient* netClient, uint64_t accountId);

    void onLoad() override;
    void update(float deltaTime) override;
    void render() override;

    // Input handling
    void handleInput(char key);
    void handleMouseClick(float x, float y);
    void handleMouseMove(float x, float y);

    // Navigation callbacks
    void setOnEnterLobby(std::function<void()> callback) { onEnterLobby = callback; }
    void setOnViewStash(std::function<void()> callback) { onViewStash = callback; }
    void setOnOpenMerchants(std::function<void()> callback) { onOpenMerchants = callback; }
    void setOnLogout(std::function<void()> callback) { onLogout = callback; }

private:
    void setupUI();
    void updateHoverStates(float mouseX, float mouseY);

    NetworkClient* networkClient;
    uint64_t accountId;
    int selectedOption;
    float animTime;
    float lastMouseX, lastMouseY;

    // Navigation callbacks
    std::function<void()> onEnterLobby;
    std::function<void()> onViewStash;
    std::function<void()> onOpenMerchants;
    std::function<void()> onLogout;

    // UI Elements
    std::shared_ptr<UIText> titleText;
    std::shared_ptr<UIText> subtitleText;
    std::shared_ptr<UIPanel> statsPanel;
    std::shared_ptr<UIPanel> buttonGridPanel;
    std::shared_ptr<UIPanel> infoPanel;
    std::shared_ptr<UIText> infoText;

    std::shared_ptr<UIButton> lobbyButton;
    std::shared_ptr<UIButton> stashButton;
    std::shared_ptr<UIButton> merchantsButton;
    std::shared_ptr<UIButton> logoutButton;
};
