#include "LoginScene.h"
#include "../../common/NetworkProtocol.h"
#include <cmath>

LoginScene::LoginScene(NetworkClient* netClient)
    : Scene("LoginScene"), networkClient(netClient), accountId(0),
      mode(Mode::LOGIN), selectedField(0), waitingForResponse(false),
      animTime(0.0f), lastMouseX(0.0f), lastMouseY(0.0f) {
}

void LoginScene::onLoad() {
    Scene::onLoad();
    setupUI();
}

void LoginScene::setupUI() {
    // Clear existing UI
    rootObjects.clear();

    // Background panel (full screen)
    auto background = std::make_shared<UIPanel>("Background");
    background->setColor(Color(0.08f, 0.08f, 0.12f, 1.0f));
    background->setSize(2.0f, 2.0f);
    background->getTransform().x = -1.0f;
    background->getTransform().y = -1.0f;
    background->setShadow(false);
    background->setDrawBorder(false);
    background->setTag("background");
    addRootObject(background);

    // Title text
    titleText = std::make_shared<UIText>("Title", "EXTRACTION SHOOTER");
    titleText->setFontSize(2.2f);
    titleText->setAlignment(TextAlign::CENTER);
    titleText->setColor(Color(1.0f, 0.8f, 0.4f, 1.0f));
    titleText->getTransform().x = -0.5f;
    titleText->getTransform().y = 0.75f;
    titleText->setSize(1.0f, 0.1f);
    titleText->setShadow(true, 0.015f, 0.015f, 0.5f);
    titleText->setTag("title");
    addRootObject(titleText);

    // Main panel
    float panelW = 0.65f;
    float panelH = (mode == Mode::LOGIN) ? 0.8f : 1.0f;
    mainPanel = std::make_shared<UIPanel>("MainPanel");
    mainPanel->setSize(panelW, panelH);
    mainPanel->getTransform().x = -panelW / 2.0f;
    mainPanel->getTransform().y = -0.35f;
    mainPanel->setColor(Color(0.12f, 0.12f, 0.16f, 0.95f));
    mainPanel->setShadow(true, 0.02f, 0.02f, 0.5f);
    mainPanel->setDrawBorder(true);
    mainPanel->setTag("mainPanel");
    addRootObject(mainPanel);

    // Mode title
    auto modeTitle = std::make_shared<UIText>("ModeTitle",
        mode == Mode::LOGIN ? "LOGIN" : "REGISTER");
    modeTitle->setFontSize(1.6f);
    modeTitle->setAlignment(TextAlign::CENTER);
    modeTitle->setColor(Color(0.85f, 0.85f, 0.85f, 1.0f));
    modeTitle->getTransform().x = 0.0f;
    modeTitle->getTransform().y = panelH - 0.15f;
    modeTitle->setSize(panelW, 0.1f);
    mainPanel->addChild(modeTitle);

    float fieldY = panelH - 0.3f;
    float fieldSpacing = 0.15f;

    // Username
    usernameLabel = std::make_shared<UIText>("UsernameLabel", "Username:");
    usernameLabel->setFontSize(1.0f);
    usernameLabel->setColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
    usernameLabel->getTransform().x = 0.05f;
    usernameLabel->getTransform().y = fieldY;
    mainPanel->addChild(usernameLabel);

    usernameField = std::make_shared<UIText>("UsernameField", username.empty() ? "_" : username);
    usernameField->setFontSize(1.0f);
    usernameField->setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    usernameField->getTransform().x = 0.25f;
    usernameField->getTransform().y = fieldY;
    mainPanel->addChild(usernameField);

    fieldY -= fieldSpacing;

    // Password
    passwordLabel = std::make_shared<UIText>("PasswordLabel", "Password:");
    passwordLabel->setFontSize(1.0f);
    passwordLabel->setColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
    passwordLabel->getTransform().x = 0.05f;
    passwordLabel->getTransform().y = fieldY;
    mainPanel->addChild(passwordLabel);

    std::string passDisplay;
    for (size_t i = 0; i < password.length(); i++) passDisplay += "*";
    if (passDisplay.empty()) passDisplay = "_";

    passwordField = std::make_shared<UIText>("PasswordField", passDisplay);
    passwordField->setFontSize(1.0f);
    passwordField->setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    passwordField->getTransform().x = 0.25f;
    passwordField->getTransform().y = fieldY;
    mainPanel->addChild(passwordField);

    fieldY -= fieldSpacing;

    // Email (register only)
    if (mode == Mode::REGISTER) {
        emailLabel = std::make_shared<UIText>("EmailLabel", "Email:");
        emailLabel->setFontSize(1.0f);
        emailLabel->setColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
        emailLabel->getTransform().x = 0.05f;
        emailLabel->getTransform().y = fieldY;
        mainPanel->addChild(emailLabel);

        emailField = std::make_shared<UIText>("EmailField", email.empty() ? "_" : email);
        emailField->setFontSize(1.0f);
        emailField->setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
        emailField->getTransform().x = 0.25f;
        emailField->getTransform().y = fieldY;
        mainPanel->addChild(emailField);

        fieldY -= fieldSpacing;
    }

    fieldY -= 0.05f;

    // Submit button
    float btnW = 0.5f;
    float btnH = 0.1f;
    submitButton = std::make_shared<UIButton>("SubmitButton",
        mode == Mode::LOGIN ? "LOGIN" : "REGISTER");
    submitButton->setSize(btnW, btnH);
    submitButton->getTransform().x = (panelW - btnW) / 2.0f;
    submitButton->getTransform().y = fieldY;
    submitButton->setTag("submitButton");
    submitButton->setOnClick([this]() {
        if (mode == Mode::LOGIN) attemptLogin();
        else attemptRegister();
    });
    mainPanel->addChild(submitButton);

    fieldY -= btnH + 0.05f;

    // Switch mode button
    switchModeButton = std::make_shared<UIButton>("SwitchModeButton",
        mode == Mode::LOGIN ? "Create Account" : "Back to Login");
    switchModeButton->setSize(btnW, btnH * 0.8f);
    switchModeButton->getTransform().x = (panelW - btnW) / 2.0f;
    switchModeButton->getTransform().y = fieldY;
    switchModeButton->setNormalColor(Color(0.1f, 0.1f, 0.15f, 0.95f));
    switchModeButton->setHoverColor(Color(0.18f, 0.18f, 0.22f, 0.95f));
    switchModeButton->setTag("switchModeButton");
    switchModeButton->setOnClick([this]() {
        switchMode(mode == Mode::LOGIN ? Mode::REGISTER : Mode::LOGIN);
    });
    mainPanel->addChild(switchModeButton);

    // Error text
    errorText = std::make_shared<UIText>("ErrorText", errorMessage);
    errorText->setFontSize(0.9f);
    errorText->setAlignment(TextAlign::CENTER);
    errorText->setColor(Color(1.0f, 0.3f, 0.3f, 1.0f));
    errorText->getTransform().x = 0.0f;
    errorText->getTransform().y = 0.15f;
    errorText->setSize(panelW, 0.1f);
    mainPanel->addChild(errorText);

    // Status text
    statusText = std::make_shared<UIText>("StatusText", statusMessage);
    statusText->setFontSize(0.9f);
    statusText->setAlignment(TextAlign::CENTER);
    statusText->setColor(Color(0.4f, 0.8f, 1.0f, 1.0f));
    statusText->getTransform().x = 0.0f;
    statusText->getTransform().y = 0.05f;
    statusText->setSize(panelW, 0.1f);
    mainPanel->addChild(statusText);
}

void LoginScene::switchMode(Mode newMode) {
    mode = newMode;
    errorMessage.clear();
    statusMessage.clear();
    setupUI();
}

void LoginScene::attemptLogin() {
    if (username.empty() || password.empty()) {
        errorMessage = "Please enter username and password";
        errorText->setText(errorMessage);
        return;
    }

    waitingForResponse = true;
    statusMessage = "Connecting...";
    statusText->setText(statusMessage);

    LoginRequest req;
    strncpy_s(req.username, username.c_str(), sizeof(req.username) - 1);
    strncpy_s(req.password, password.c_str(), sizeof(req.password) - 1);
    networkClient->sendPacket(PacketType::LOGIN_REQUEST, &req, static_cast<uint32_t>(sizeof(req)));
}

void LoginScene::attemptRegister() {
    if (username.empty() || password.empty() || email.empty()) {
        errorMessage = "Please fill all fields";
        errorText->setText(errorMessage);
        return;
    }

    if (email.find('@') == std::string::npos) {
        errorMessage = "Invalid email address";
        errorText->setText(errorMessage);
        return;
    }

    waitingForResponse = true;
    statusMessage = "Creating account...";
    statusText->setText(statusMessage);

    RegisterRequest req;
    strncpy_s(req.username, username.c_str(), sizeof(req.username) - 1);
    strncpy_s(req.password, password.c_str(), sizeof(req.password) - 1);
    strncpy_s(req.email, email.c_str(), sizeof(req.email) - 1);
    networkClient->sendPacket(PacketType::REGISTER_REQUEST, &req, static_cast<uint32_t>(sizeof(req)));
}

void LoginScene::processNetworkPackets() {
    Packet packet;
    while (networkClient->receivePacket(&packet)) {
        if (packet.type == PacketType::LOGIN_RESPONSE) {
            LoginResponse* resp = (LoginResponse*)packet.data;
            waitingForResponse = false;
            if (resp->success) {
                accountId = resp->accountId;
                statusMessage = "Login successful!";
                // Scene transition will be handled by main application
            } else {
                errorMessage = resp->message;
                errorText->setText(errorMessage);
                statusMessage.clear();
                statusText->setText("");
            }
        }
        else if (packet.type == PacketType::REGISTER_RESPONSE) {
            RegisterResponse* resp = (RegisterResponse*)packet.data;
            waitingForResponse = false;
            if (resp->success) {
                statusMessage = "Registration successful! Please login.";
                statusText->setText(statusMessage);
                switchMode(Mode::LOGIN);
            } else {
                errorMessage = resp->message;
                errorText->setText(errorMessage);
            }
        }
    }
}

void LoginScene::update(float deltaTime) {
    animTime += deltaTime;

    // Animate title
    float pulse = 0.85f + 0.15f * std::sin(animTime * 2.0f);
    titleText->setColor(Color(pulse, pulse * 0.8f, pulse * 0.4f, 1.0f));

    // Animate status dots
    if (waitingForResponse) {
        int dotCount = (int)(animTime * 2.0f) % 4;
        std::string dots(dotCount, '.');
        statusText->setText(statusMessage + dots);
    }

    processNetworkPackets();

    Scene::update(deltaTime);
}

void LoginScene::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    Scene::render();
}

void LoginScene::handleInput(char key) {
    if (waitingForResponse) return;

    // Tab - switch field
    if (key == '\t' || key == 9) {
        selectedField = (selectedField + 1) % (mode == Mode::LOGIN ? 2 : 3);
        return;
    }

    // Backspace
    if (key == '\b' || key == 8) {
        if (selectedField == 0 && !username.empty()) {
            username.pop_back();
            usernameField->setText(username.empty() ? "_" : username);
        }
        else if (selectedField == 1 && !password.empty()) {
            password.pop_back();
            std::string passDisplay;
            for (size_t i = 0; i < password.length(); i++) passDisplay += "*";
            passwordField->setText(passDisplay.empty() ? "_" : passDisplay);
        }
        else if (selectedField == 2 && !email.empty()) {
            email.pop_back();
            emailField->setText(email.empty() ? "_" : email);
        }
        return;
    }

    // Enter - submit
    if (key == '\r' || key == '\n' || key == 13) {
        if (mode == Mode::LOGIN) attemptLogin();
        else attemptRegister();
        return;
    }

    // Alphanumeric input
    if ((key >= 32 && key <= 126)) {
        if (selectedField == 0 && username.length() < 32) {
            username += key;
            usernameField->setText(username);
        }
        else if (selectedField == 1 && password.length() < 32) {
            password += key;
            std::string passDisplay;
            for (size_t i = 0; i < password.length(); i++) passDisplay += "*";
            passwordField->setText(passDisplay);
        }
        else if (selectedField == 2 && email.length() < 64) {
            email += key;
            emailField->setText(email);
        }
    }
}

void LoginScene::handleMouseClick(float x, float y) {
    // Check button clicks
    for (auto& obj : rootObjects) {
        auto checkButton = [x, y](std::shared_ptr<GameObject> obj) -> bool {
            auto button = std::dynamic_pointer_cast<UIButton>(obj);
            if (button && button->isActive() && button->containsPoint(x, y)) {
                button->onClick();
                return true;
            }
            return false;
        };

        if (checkButton(obj)) return;

        // Check children recursively
        for (auto& child : obj->getChildren()) {
            if (checkButton(child)) return;
            for (auto& grandchild : child->getChildren()) {
                if (checkButton(grandchild)) return;
            }
        }
    }
}

void LoginScene::handleMouseMove(float x, float y) {
    lastMouseX = x;
    lastMouseY = y;
    updateHoverStates(x, y);
}

void LoginScene::updateHoverStates(float mouseX, float mouseY) {
    // Update hover states for all buttons
    auto updateButton = [mouseX, mouseY](std::shared_ptr<GameObject> obj) {
        auto button = std::dynamic_pointer_cast<UIButton>(obj);
        if (button && button->isActive()) {
            bool wasHovered = button->isHovered();
            bool nowHovered = button->containsPoint(mouseX, mouseY);

            if (nowHovered != wasHovered) {
                button->setHovered(nowHovered);
                if (nowHovered) button->onMouseEnter();
                else button->onMouseExit();
            }
        }
    };

    for (auto& obj : rootObjects) {
        updateButton(obj);
        for (auto& child : obj->getChildren()) {
            updateButton(child);
            for (auto& grandchild : child->getChildren()) {
                updateButton(grandchild);
            }
        }
    }
}
