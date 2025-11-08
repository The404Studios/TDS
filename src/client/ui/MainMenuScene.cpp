#include "MainMenuScene.h"
#include <cmath>

MainMenuScene::MainMenuScene(NetworkClient* netClient, uint64_t accountId)
    : Scene("MainMenuScene"), networkClient(netClient), accountId(accountId),
      selectedOption(0), animTime(0.0f), lastMouseX(0.0f), lastMouseY(0.0f) {
}

void MainMenuScene::onLoad() {
    Scene::onLoad();
    setupUI();
}

void MainMenuScene::setupUI() {
    // Clear existing UI
    rootObjects.clear();

    // Background
    auto background = std::make_shared<UIPanel>("Background");
    background->setColor(Color(0.08f, 0.08f, 0.12f, 1.0f));
    background->setSize(2.0f, 2.0f);
    background->getTransform().x = -1.0f;
    background->getTransform().y = -1.0f;
    background->setShadow(false);
    background->setDrawBorder(false);
    background->setTag("background");
    addRootObject(background);

    // Title
    titleText = std::make_shared<UIText>("Title", "EXTRACTION SHOOTER");
    titleText->setFontSize(2.2f);
    titleText->setAlignment(TextAlign::CENTER);
    titleText->setColor(Color(1.0f, 0.85f, 0.5f, 1.0f));
    titleText->getTransform().x = -0.5f;
    titleText->getTransform().y = 0.8f;
    titleText->setSize(1.0f, 0.1f);
    titleText->setShadow(true, 0.015f, 0.015f, 0.5f);
    titleText->setTag("title");
    addRootObject(titleText);

    // Subtitle
    subtitleText = std::make_shared<UIText>("Subtitle", "MAIN MENU");
    subtitleText->setFontSize(1.4f);
    subtitleText->setAlignment(TextAlign::CENTER);
    subtitleText->setColor(Color(0.65f, 0.65f, 0.65f, 1.0f));
    subtitleText->getTransform().x = -0.5f;
    subtitleText->getTransform().y = 0.65f;
    subtitleText->setSize(1.0f, 0.1f);
    titleText->setTag("subtitle");
    addRootObject(subtitleText);

    // Player stats panel (top right)
    float statsW = 0.5f;
    float statsH = 0.45f;
    statsPanel = std::make_shared<UIPanel>("StatsPanel");
    statsPanel->setSize(statsW, statsH);
    statsPanel->getTransform().x = 0.4f;
    statsPanel->getTransform().y = 0.35f;
    statsPanel->setColor(Color(0.12f, 0.12f, 0.16f, 0.95f));
    statsPanel->setShadow(true, 0.015f, 0.015f, 0.4f);
    statsPanel->setDrawBorder(true);
    statsPanel->setTag("statsPanel");
    addRootObject(statsPanel);

    // Stats content
    auto statsTitle = std::make_shared<UIText>("StatsTitle", "PLAYER STATS");
    statsTitle->setFontSize(1.2f);
    statsTitle->setAlignment(TextAlign::CENTER);
    statsTitle->setColor(Color(0.9f, 0.8f, 0.5f, 1.0f));
    statsTitle->getTransform().x = 0.0f;
    statsTitle->getTransform().y = statsH - 0.1f;
    statsTitle->setSize(statsW, 0.1f);
    statsPanel->addChild(statsTitle);

    float statY = statsH - 0.2f;
    float statSpacing = 0.08f;

    auto addStat = [&](const std::string& label, const std::string& value) {
        auto labelText = std::make_shared<UIText>(label + "Label", label);
        labelText->setFontSize(0.9f);
        labelText->setColor(Color(0.6f, 0.6f, 0.6f, 1.0f));
        labelText->getTransform().x = 0.05f;
        labelText->getTransform().y = statY;
        statsPanel->addChild(labelText);

        auto valueText = std::make_shared<UIText>(label + "Value", value);
        valueText->setFontSize(0.9f);
        valueText->setColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
        valueText->getTransform().x = 0.25f;
        valueText->getTransform().y = statY;
        statsPanel->addChild(valueText);

        statY -= statSpacing;
    };

    addStat("Level:", "15");
    addStat("Roubles:", "450,320");
    addStat("Raids:", "247");
    addStat("K/D Ratio:", "2.84");

    // Button grid (2x2)
    float btnW = 0.35f;
    float btnH = 0.13f;
    float gridGap = 0.03f;
    float totalW = btnW * 2 + gridGap;
    float gridX = -totalW / 2.0f;
    float gridY = 0.1f;

    // Container panel for buttons (for easy management)
    buttonGridPanel = std::make_shared<UIPanel>("ButtonGrid");
    buttonGridPanel->setSize(totalW, btnH * 2 + gridGap);
    buttonGridPanel->getTransform().x = gridX;
    buttonGridPanel->getTransform().y = gridY - (btnH * 2 + gridGap);
    buttonGridPanel->setColor(Color(0, 0, 0, 0)); // Transparent
    buttonGridPanel->setShadow(false);
    buttonGridPanel->setDrawBorder(false);
    buttonGridPanel->setTag("buttonGrid");
    addRootObject(buttonGridPanel);

    // ENTER LOBBY button (top left)
    lobbyButton = std::make_shared<UIButton>("LobbyButton", "ENTER LOBBY");
    lobbyButton->setSize(btnW, btnH);
    lobbyButton->getTransform().x = 0.0f;
    lobbyButton->getTransform().y = btnH + gridGap;
    lobbyButton->setTag("lobbyButton");
    lobbyButton->setOnClick([this]() {
        if (onEnterLobby) onEnterLobby();
    });
    buttonGridPanel->addChild(lobbyButton);

    // VIEW STASH button (top right)
    stashButton = std::make_shared<UIButton>("StashButton", "VIEW STASH");
    stashButton->setSize(btnW, btnH);
    stashButton->getTransform().x = btnW + gridGap;
    stashButton->getTransform().y = btnH + gridGap;
    stashButton->setTag("stashButton");
    stashButton->setOnClick([this]() {
        if (onViewStash) onViewStash();
    });
    buttonGridPanel->addChild(stashButton);

    // MERCHANTS button (bottom left)
    merchantsButton = std::make_shared<UIButton>("MerchantsButton", "MERCHANTS");
    merchantsButton->setSize(btnW, btnH);
    merchantsButton->getTransform().x = 0.0f;
    merchantsButton->getTransform().y = 0.0f;
    merchantsButton->setTag("merchantsButton");
    merchantsButton->setOnClick([this]() {
        if (onOpenMerchants) onOpenMerchants();
    });
    buttonGridPanel->addChild(merchantsButton);

    // LOGOUT button (bottom right)
    logoutButton = std::make_shared<UIButton>("LogoutButton", "LOGOUT");
    logoutButton->setSize(btnW, btnH);
    logoutButton->getTransform().x = btnW + gridGap;
    logoutButton->getTransform().y = 0.0f;
    logoutButton->setTag("logoutButton");
    logoutButton->setNormalColor(Color(0.2f, 0.1f, 0.1f, 0.95f));
    logoutButton->setHoverColor(Color(0.35f, 0.15f, 0.15f, 0.95f));
    logoutButton->setOnClick([this]() {
        if (onLogout) onLogout();
    });
    buttonGridPanel->addChild(logoutButton);

    // Info panel (bottom)
    float infoW = 1.6f;
    float infoH = 0.15f;
    infoPanel = std::make_shared<UIPanel>("InfoPanel");
    infoPanel->setSize(infoW, infoH);
    infoPanel->getTransform().x = -infoW / 2.0f;
    infoPanel->getTransform().y = -0.85f;
    infoPanel->setColor(Color(0.1f, 0.1f, 0.14f, 0.9f));
    infoPanel->setShadow(true, 0.01f, 0.01f, 0.3f);
    infoPanel->setDrawBorder(true);
    infoPanel->setTag("infoPanel");
    addRootObject(infoPanel);

    infoText = std::make_shared<UIText>("InfoText", "Select an option to continue");
    infoText->setFontSize(1.0f);
    infoText->setAlignment(TextAlign::CENTER);
    infoText->setColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
    infoText->getTransform().x = 0.0f;
    infoText->getTransform().y = infoH / 2.0f - 0.03f;
    infoText->setSize(infoW, infoH);
    infoPanel->addChild(infoText);

    // Version text
    auto versionText = std::make_shared<UIText>("VersionText", "v1.0.0 Alpha");
    versionText->setFontSize(0.7f);
    versionText->setColor(Color(0.4f, 0.4f, 0.4f, 1.0f));
    versionText->getTransform().x = -0.88f;
    versionText->getTransform().y = -0.92f;
    addRootObject(versionText);
}

void MainMenuScene::update(float deltaTime) {
    animTime += deltaTime;

    // Animate title
    float pulse = 0.85f + 0.15f * std::sinf(animTime * 2.0f);
    titleText->setColor(Color(pulse, pulse * 0.85f, pulse * 0.5f, 1.0f));

    // Update info text based on hovered button
    if (lobbyButton && lobbyButton->isHovered()) {
        infoText->setText("Join or create a lobby to play with others");
    } else if (stashButton && stashButton->isHovered()) {
        infoText->setText("Manage your inventory and equipment");
    } else if (merchantsButton && merchantsButton->isHovered()) {
        infoText->setText("Buy and sell items with traders");
    } else if (logoutButton && logoutButton->isHovered()) {
        infoText->setText("Return to login screen");
    } else {
        infoText->setText("Select an option to continue");
    }

    Scene::update(deltaTime);
}

void MainMenuScene::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    Scene::render();
}

void MainMenuScene::handleInput(char key) {
    // Number keys 1-4
    if (key >= '1' && key <= '4') {
        selectedOption = key - '1';

        switch (selectedOption) {
            case 0: if (onEnterLobby) onEnterLobby(); break;
            case 1: if (onViewStash) onViewStash(); break;
            case 2: if (onOpenMerchants) onOpenMerchants(); break;
            case 3: if (onLogout) onLogout(); break;
        }
    }

    // Arrow keys and WASD
    if (key == 'w' || key == 'W' || key == 72) { // Up
        selectedOption = (selectedOption - 2 + 4) % 4;
    }
    if (key == 's' || key == 'S' || key == 80) { // Down
        selectedOption = (selectedOption + 2) % 4;
    }
    if (key == 'a' || key == 'A' || key == 75) { // Left
        if (selectedOption % 2 == 1) selectedOption--;
    }
    if (key == 'd' || key == 'D' || key == 77) { // Right
        if (selectedOption % 2 == 0) selectedOption++;
    }

    // Enter - activate selected
    if (key == '\r' || key == '\n' || key == 13) {
        switch (selectedOption) {
            case 0: if (onEnterLobby) onEnterLobby(); break;
            case 1: if (onViewStash) onViewStash(); break;
            case 2: if (onOpenMerchants) onOpenMerchants(); break;
            case 3: if (onLogout) onLogout(); break;
        }
    }
}

void MainMenuScene::handleMouseClick(float x, float y) {
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

        // Check children recursively (up to 3 levels deep)
        for (auto& child : obj->getChildren()) {
            if (checkButton(child)) return;
            for (auto& grandchild : child->getChildren()) {
                if (checkButton(grandchild)) return;
                for (auto& greatGrandchild : grandchild->getChildren()) {
                    if (checkButton(greatGrandchild)) return;
                }
            }
        }
    }
}

void MainMenuScene::handleMouseMove(float x, float y) {
    lastMouseX = x;
    lastMouseY = y;
    updateHoverStates(x, y);
}

void MainMenuScene::updateHoverStates(float mouseX, float mouseY) {
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
                for (auto& greatGrandchild : grandchild->getChildren()) {
                    updateButton(greatGrandchild);
                }
            }
        }
    }
}
