#pragma once

#include "../../engine/scene/IScene.h"
#include "../../client/network/NetworkClient.h"
#include "../../client/ui/UIPanel.h"
#include "../../client/ui/UIButton.h"
#include "../../client/ui/UIText.h"
#include <memory>
#include <vector>

/**
 * MenuScene - Main menu and matchmaking
 *
 * Responsibilities:
 * - Display main menu UI
 * - Handle login (if not already logged in)
 * - Start matchmaking
 * - Switch to RaidScene when match is found
 *
 * This scene is lightweight - only UI rendering, no physics/particles.
 */
class MenuScene : public IScene {
public:
    MenuScene(NetworkClient* netClient);
    ~MenuScene() override;

    std::string name() const override { return "Menu"; }

    bool onEnter() override;
    void onExit() override;

    void fixedUpdate(float dt) override;
    void update(float dt) override;
    void render() override;

    // Input handling
    void handleInput(char key);
    void handleMouseClick(float x, float y);
    void handleMouseMove(float x, float y);

private:
    void setupUI();
    void onPlayClicked();
    void onStashClicked();
    void onSettingsClicked();
    void onQuitClicked();

    void processNetworkPackets();

    NetworkClient* networkClient;

    // UI Elements
    std::shared_ptr<UIPanel> backgroundPanel;
    std::shared_ptr<UIText> titleText;
    std::shared_ptr<UIButton> playButton;
    std::shared_ptr<UIButton> stashButton;
    std::shared_ptr<UIButton> settingsButton;
    std::shared_ptr<UIButton> quitButton;
    std::shared_ptr<UIText> statusText;

    // State
    bool inMatchmaking;
    float matchmakingTime;
    float mouseX, mouseY;
};
