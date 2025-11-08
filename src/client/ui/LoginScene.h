#pragma once

#include "Scene.h"
#include "UIPanel.h"
#include "UIButton.h"
#include "UIText.h"
#include "../network/NetworkClient.h"
#include <memory>

// Login Scene - Handles login and registration
class LoginScene : public Scene {
public:
    LoginScene(NetworkClient* netClient);

    void onLoad() override;
    void update(float deltaTime) override;
    void render() override;

    // Input handling
    void handleInput(char key);
    void handleMouseClick(float x, float y);
    void handleMouseMove(float x, float y);

    // Get account ID after successful login
    uint64_t getAccountId() const { return accountId; }

private:
    enum class Mode { LOGIN, REGISTER };

    void setupUI();
    void switchMode(Mode newMode);
    void attemptLogin();
    void attemptRegister();
    void processNetworkPackets();
    void updateHoverStates(float mouseX, float mouseY);

    NetworkClient* networkClient;
    uint64_t accountId;
    Mode mode;
    int selectedField;
    bool waitingForResponse;

    std::string username;
    std::string password;
    std::string email;
    std::string errorMessage;
    std::string statusMessage;

    float animTime;
    float lastMouseX, lastMouseY;

    // UI Elements (tagged for easy lookup)
    std::shared_ptr<UIPanel> mainPanel;
    std::shared_ptr<UIText> titleText;
    std::shared_ptr<UIText> errorText;
    std::shared_ptr<UIText> statusText;
    std::shared_ptr<UIButton> submitButton;
    std::shared_ptr<UIButton> switchModeButton;
    std::shared_ptr<UIText> usernameLabel;
    std::shared_ptr<UIText> usernameField;
    std::shared_ptr<UIText> passwordLabel;
    std::shared_ptr<UIText> passwordField;
    std::shared_ptr<UIText> emailLabel;
    std::shared_ptr<UIText> emailField;
};
