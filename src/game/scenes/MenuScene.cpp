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
    playButton.reset();
    stashButton.reset();
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
    playButton->update(dt);
    stashButton->update(dt);
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
    if (playButton) playButton->render();
    if (stashButton) stashButton->render();
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

    // Play button
    playButton = std::make_shared<UIButton>("PlayButton", "FIND MATCH");
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

    // Settings button
    settingsButton = std::make_shared<UIButton>("SettingsButton", "SETTINGS");
    settingsButton->setSize(0.4f, 0.1f);
    settingsButton->getTransform().x = -0.2f;
    settingsButton->getTransform().y = -0.1f;
    settingsButton->setOnClick([this]() { onSettingsClicked(); });

    // Quit button
    quitButton = std::make_shared<UIButton>("QuitButton", "QUIT");
    quitButton->setSize(0.4f, 0.1f);
    quitButton->getTransform().x = -0.2f;
    quitButton->getTransform().y = -0.25f;
    quitButton->setOnClick([this]() { onQuitClicked(); });

    // Status text
    statusText = std::make_shared<UIText>("StatusText");
    statusText->setText("");
    statusText->setFontSize(1.0f);
    statusText->setColor(Color(0.7f, 0.7f, 0.7f, 1.0f));
    statusText->getTransform().x = -0.2f;
    statusText->getTransform().y = -0.5f;
}

void MenuScene::onPlayClicked() {
    std::cout << "[MenuScene] Play clicked - starting matchmaking" << std::endl;

    if (!inMatchmaking) {
        // Start matchmaking
        LobbyReady startQueue;
        startQueue.ready = true;
        networkClient->sendPacket(PacketType::LOBBY_START_QUEUE, &startQueue, static_cast<uint32_t>(sizeof(startQueue)));

        inMatchmaking = true;
        matchmakingTime = 0.0f;
        statusText->setText("Searching for match...");
    }
}

void MenuScene::onStashClicked() {
    std::cout << "[MenuScene] Stash clicked" << std::endl;
    // TODO: Switch to stash scene
    statusText->setText("Stash not implemented yet");
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
    if (playButton && playButton->containsPoint(x, y)) {
        playButton->onClick();
    } else if (stashButton && stashButton->containsPoint(x, y)) {
        stashButton->onClick();
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
    if (playButton) {
        playButton->setHovered(playButton->containsPoint(x, y));
    }
    if (stashButton) {
        stashButton->setHovered(stashButton->containsPoint(x, y));
    }
    if (settingsButton) {
        settingsButton->setHovered(settingsButton->containsPoint(x, y));
    }
    if (quitButton) {
        quitButton->setHovered(quitButton->containsPoint(x, y));
    }
}
