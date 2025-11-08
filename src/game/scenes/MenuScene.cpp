#include "MenuScene.h"
#include "../../engine/GameEngine.h"
#include "../../common/NetworkProtocol.h"
#include <iostream>
#include <cmath>

MenuScene::MenuScene(NetworkClient* netClient)
    : networkClient(netClient),
      inMatchmaking(false),
      matchmakingTime(0.0f),
      mouseX(0.0f),
      mouseY(0.0f) {
}

MenuScene::~MenuScene() {
}

bool MenuScene::onEnter() {
    std::cout << "[MenuScene] Entering menu" << std::endl;

    setupUI();

    // Load menu music (example)
    // auto audio = ENGINE.getAudioEngine();
    // auto menuMusic = ENGINE.getResourceManager()->loadAudio("assets/audio/menu_music.wav");
    // menuMusicSource->setClip(menuMusic);
    // menuMusicSource->setLoop(true);
    // menuMusicSource->play();

    return true;
}

void MenuScene::onExit() {
    std::cout << "[MenuScene] Exiting menu" << std::endl;

    // Stop matchmaking if active
    if (inMatchmaking) {
        LobbyReady stopQueue;
        stopQueue.ready = false;
        networkClient->sendPacket(PacketType::LOBBY_STOP_QUEUE, &stopQueue, static_cast<uint32_t>(sizeof(stopQueue)));
        inMatchmaking = false;
    }

    // Stop menu music
    // menuMusicSource->stop();

    // Cleanup UI
    backgroundPanel.reset();
    titleText.reset();
    lobbyButton.reset();
    playButton.reset();
    stashButton.reset();
    merchantButton.reset();
    settingsButton.reset();
    quitButton.reset();
    statusText.reset();
}

void MenuScene::fixedUpdate(float dt) {
    // No physics/networking in menu
    // Network packets are processed in update()
}

void MenuScene::update(float dt) {
    // Process network packets
    processNetworkPackets();

    // Update matchmaking timer
    if (inMatchmaking) {
        matchmakingTime += dt;

        // Update status text
        int dots = (int)(matchmakingTime * 2.0f) % 4;
        std::string status = "Searching for match";
        for (int i = 0; i < dots; i++) {
            status += ".";
        }
        statusText->setText(status);
    }

    // Update UI hover states
    lobbyButton->update(dt);
    playButton->update(dt);
    stashButton->update(dt);
    merchantButton->update(dt);
    settingsButton->update(dt);
    quitButton->update(dt);
}

void MenuScene::render() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Render UI
    if (backgroundPanel) backgroundPanel->render();
    if (titleText) titleText->render();
    if (lobbyButton) lobbyButton->render();
    if (playButton) playButton->render();
    if (stashButton) stashButton->render();
    if (merchantButton) merchantButton->render();
    if (settingsButton) settingsButton->render();
    if (quitButton) quitButton->render();
    if (statusText) statusText->render();
}

void MenuScene::setupUI() {
    // Background
    backgroundPanel = std::make_shared<UIPanel>("Background");
    backgroundPanel->setColor(Color(0.05f, 0.05f, 0.1f, 1.0f));
    backgroundPanel->setSize(2.0f, 2.0f);
    backgroundPanel->getTransform().x = -1.0f;
    backgroundPanel->getTransform().y = -1.0f;
    backgroundPanel->setShadow(false);
    backgroundPanel->setDrawBorder(false);

    // Title
    titleText = std::make_shared<UIText>("TitleText");
    titleText->setText("EXTRACTION SHOOTER");
    titleText->setFontSize(2.0f);
    titleText->setColor(Color(0.9f, 0.7f, 0.3f, 1.0f));
    titleText->getTransform().x = -0.4f;
    titleText->getTransform().y = 0.6f;

    // Lobby button (party matchmaking)
    lobbyButton = std::make_shared<UIButton>("LobbyButton", "LOBBY (PARTY)");
    lobbyButton->setSize(0.4f, 0.1f);
    lobbyButton->getTransform().x = -0.2f;
    lobbyButton->getTransform().y = 0.35f;
    lobbyButton->setOnClick([this]() { onLobbyClicked(); });

    // Play button (solo queue)
    playButton = std::make_shared<UIButton>("PlayButton", "SOLO QUEUE");
    playButton->setSize(0.4f, 0.1f);
    playButton->getTransform().x = -0.2f;
    playButton->getTransform().y = 0.2f;
    playButton->setOnClick([this]() { onPlayClicked(); });

    // Stash button
    stashButton = std::make_shared<UIButton>("StashButton", "STASH");
    stashButton->setSize(0.4f, 0.1f);
    stashButton->getTransform().x = -0.2f;
    stashButton->getTransform().y = 0.05f;
    stashButton->setOnClick([this]() { onStashClicked(); });

    // Merchant button
    merchantButton = std::make_shared<UIButton>("MerchantButton", "TRADERS");
    merchantButton->setSize(0.4f, 0.1f);
    merchantButton->getTransform().x = -0.2f;
    merchantButton->getTransform().y = -0.1f;
    merchantButton->setOnClick([this]() { onMerchantClicked(); });

    // Settings button
    settingsButton = std::make_shared<UIButton>("SettingsButton", "SETTINGS");
    settingsButton->setSize(0.4f, 0.1f);
    settingsButton->getTransform().x = -0.2f;
    settingsButton->getTransform().y = -0.25f;
    settingsButton->setOnClick([this]() { onSettingsClicked(); });

    // Quit button
    quitButton = std::make_shared<UIButton>("QuitButton", "QUIT");
    quitButton->setSize(0.4f, 0.1f);
    quitButton->getTransform().x = -0.2f;
    quitButton->getTransform().y = -0.4f;
    quitButton->setOnClick([this]() { onQuitClicked(); });

    // Status text
    statusText = std::make_shared<UIText>("StatusText");
    statusText->setText("");
    statusText->setFontSize(1.0f);
    statusText->setColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
    statusText->getTransform().x = -0.2f;
    statusText->getTransform().y = -0.5f;
}

void MenuScene::onLobbyClicked() {
    std::cout << "[MenuScene] Lobby clicked - switching to lobby scene for party matchmaking" << std::endl;
    ENGINE.getSceneManager()->switchTo("lobby");
}

void MenuScene::onPlayClicked() {
    std::cout << "[MenuScene] Play clicked - starting solo matchmaking" << std::endl;

    if (!inMatchmaking) {
        // Start matchmaking
        LobbyReady startQueue;
        startQueue.ready = true;
        networkClient->sendPacket(PacketType::LOBBY_START_QUEUE, &startQueue, static_cast<uint32_t>(sizeof(startQueue)));

        inMatchmaking = true;
        matchmakingTime = 0.0f;
        statusText->setText("Searching for 100-player match (solo)...");
    }
}

void MenuScene::onStashClicked() {
    std::cout << "[MenuScene] Stash clicked - switching to stash scene" << std::endl;
    ENGINE.getSceneManager()->switchTo("stash");
}

void MenuScene::onMerchantClicked() {
    std::cout << "[MenuScene] Merchant clicked - switching to merchant scene" << std::endl;
    ENGINE.getSceneManager()->switchTo("merchant");
}

void MenuScene::onSettingsClicked() {
    std::cout << "[MenuScene] Settings clicked" << std::endl;
    // TODO: Switch to settings scene
    statusText->setText("Settings not implemented yet");
}

void MenuScene::onQuitClicked() {
    std::cout << "[MenuScene] Quit clicked" << std::endl;
    // TODO: Signal GameEngine to shutdown
    exit(0);
}

void MenuScene::processNetworkPackets() {
    while (networkClient->hasPackets()) {
        NetworkClient::ReceivedPacket packet = networkClient->getNextPacket();

        if (packet.type == PacketType::MATCH_FOUND) {
            std::cout << "[MenuScene] Match found! Switching to raid scene" << std::endl;

            inMatchmaking = false;
            statusText->setText("Match found! Loading raid...");

            // Switch to raid scene
            ENGINE.getSceneManager()->switchTo("raid");
        }
    }
}

void MenuScene::handleInput(char key) {
    // Handle keyboard input if needed
}

void MenuScene::handleMouseClick(float x, float y) {
    mouseX = x;
    mouseY = y;

    // Check button clicks
    if (lobbyButton && lobbyButton->containsPoint(x, y)) {
        lobbyButton->onClick();
    } else if (playButton && playButton->containsPoint(x, y)) {
        playButton->onClick();
    } else if (stashButton && stashButton->containsPoint(x, y)) {
        stashButton->onClick();
    } else if (merchantButton && merchantButton->containsPoint(x, y)) {
        merchantButton->onClick();
    } else if (settingsButton && settingsButton->containsPoint(x, y)) {
        settingsButton->onClick();
    } else if (quitButton && quitButton->containsPoint(x, y)) {
        quitButton->onClick();
    }
}

void MenuScene::handleMouseMove(float x, float y) {
    mouseX = x;
    mouseY = y;

    // Update button hover states
    if (lobbyButton) {
        lobbyButton->setHovered(lobbyButton->containsPoint(x, y));
    }
    if (playButton) {
        playButton->setHovered(playButton->containsPoint(x, y));
    }
    if (stashButton) {
        stashButton->setHovered(stashButton->containsPoint(x, y));
    }
    if (merchantButton) {
        merchantButton->setHovered(merchantButton->containsPoint(x, y));
    }
    if (settingsButton) {
        settingsButton->setHovered(settingsButton->containsPoint(x, y));
    }
    if (quitButton) {
        quitButton->setHovered(quitButton->containsPoint(x, y));
    }
}
