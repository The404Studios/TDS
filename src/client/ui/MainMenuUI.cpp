#include "MainMenuUI.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <gl/GL.h>
#include <cstring>
#include <cmath>

void MainMenuUI::update(float deltaTime) {
    // Update animation time
    animTime += deltaTime;

    // Process packets if needed
    while (networkClient->hasPackets()) {
        auto packet = networkClient->getNextPacket();

        switch (packet.type) {
            case PacketType::PLAYER_DATA_RESPONSE:
                handlePlayerDataResponse(packet.payload);
                break;

            default:
                break;
        }
    }
}

void MainMenuUI::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Calculate pulse effect for title
    float pulse = 0.8f + 0.2f * std::sin(animTime * 2.0f);

    // Draw animated title with glow effect
    glColor3f(pulse, pulse * 0.9f, 0.5f);
    TextRenderer::drawTextCentered("EXTRACTION SHOOTER", 0.85f, 1.8f);
    glColor3f(0.7f, 0.7f, 0.7f);
    TextRenderer::drawTextCentered("MAIN MENU", 0.7f, 1.2f);

    // Define button layout with proper spacing
    float buttonWidth = 0.5f;
    float buttonHeight = 0.12f;
    float buttonSpacing = 0.02f;
    float centerX = -buttonWidth / 2.0f;
    float startY = 0.4f;

    // Button labels
    std::vector<std::string> buttonLabels = {
        "ENTER LOBBY",
        "VIEW STASH",
        "VISIT MERCHANTS",
        "LOGOUT"
    };

    // Button descriptions for info panel
    std::vector<std::string> buttonDescriptions = {
        "Create or join a party to enter raids|Queue with friends for cooperative extraction gameplay",
        "Manage your persistent inventory|Store weapons, armor, loot, and other items here",
        "Buy and sell items from traders|5 merchants: Fence, Prapor, Therapist, Peacekeeper, Ragman",
        "Disconnect and return to login screen|Your progress will be saved automatically"
    };

    // Draw buttons with animations
    for (size_t i = 0; i < buttonLabels.size(); i++) {
        float btnY = startY - i * (buttonHeight + buttonSpacing);
        bool isHovering = isPointInRect(mouseX, mouseY, centerX, btnY, buttonWidth, buttonHeight);

        // Animate button on hover
        float hoverScale = isHovering ? 1.05f : 1.0f;
        float btnWidth = buttonWidth * hoverScale;
        float btnHeight = buttonHeight * hoverScale;
        float btnX = centerX - (btnWidth - buttonWidth) / 2.0f;
        float adjustedY = btnY - (btnHeight - buttonHeight) / 2.0f;

        // Draw button background with gradient effect
        if (isHovering) {
            glColor3f(0.25f, 0.45f, 0.75f);  // Bright blue on hover
        } else if ((int)i == selectedOption) {
            glColor3f(0.2f, 0.35f, 0.6f);   // Medium blue when selected
        } else {
            glColor3f(0.15f, 0.15f, 0.2f);  // Dark background
        }
        drawBox(btnX, adjustedY, btnWidth, btnHeight, true);

        // Draw button border with glow on hover
        if (isHovering) {
            float glowPulse = 0.7f + 0.3f * std::sin(animTime * 8.0f);
            glColor3f(glowPulse, glowPulse * 0.8f, 1.0f);
            glLineWidth(3.0f);
        } else {
            glColor3f(0.5f, 0.5f, 0.6f);
            glLineWidth(2.0f);
        }
        drawBox(btnX, adjustedY, btnWidth, btnHeight, false);
        glLineWidth(1.0f);

        // Draw button text (centered)
        glColor3f(1.0f, 1.0f, 1.0f);
        float textX = centerX + buttonWidth / 2.0f - buttonLabels[i].length() * 0.007f;
        float textY = btnY + buttonHeight / 2.0f - 0.02f;
        TextRenderer::drawText(buttonLabels[i], textX, textY, 1.0f);

        // Update selected option on hover
        if (isHovering) {
            selectedOption = static_cast<int>(i);
        }
    }

    // Draw player stats panel (left side with proper spacing)
    float panelMargin = 0.05f;
    float statsPanelX = -0.88f;
    float statsPanelY = -0.15f;
    float statsPanelW = 0.35f;
    float statsPanelH = 0.45f;

    drawPanel(statsPanelX, statsPanelY, statsPanelW, statsPanelH, "PLAYER STATS");

    // Draw stats with icons/labels
    glColor3f(0.9f, 0.9f, 0.9f);
    float statsTextX = statsPanelX + 0.02f;
    float statsY = statsPanelY + 0.35f;

    glColor3f(0.7f, 0.9f, 0.7f);
    TextRenderer::drawText("LEVEL", statsTextX, statsY);
    glColor3f(1.0f, 1.0f, 1.0f);
    TextRenderer::drawText(std::to_string(playerStats.level), statsTextX + 0.15f, statsY);

    statsY -= 0.08f;
    glColor3f(0.9f, 0.9f, 0.5f);
    TextRenderer::drawText("ROUBLES", statsTextX, statsY);
    glColor3f(1.0f, 1.0f, 1.0f);
    TextRenderer::drawText(std::to_string(playerStats.roubles), statsTextX + 0.15f, statsY);

    statsY -= 0.08f;
    glColor3f(0.6f, 0.8f, 1.0f);
    TextRenderer::drawText("RAIDS", statsTextX, statsY);
    glColor3f(1.0f, 1.0f, 1.0f);
    TextRenderer::drawText(std::to_string(playerStats.raidsCompleted), statsTextX + 0.15f, statsY);

    statsY -= 0.08f;
    glColor3f(1.0f, 0.6f, 0.6f);
    TextRenderer::drawText("K/D RATIO", statsTextX, statsY);
    glColor3f(1.0f, 1.0f, 1.0f);
    float kd = playerStats.deaths > 0 ? (float)playerStats.kills / playerStats.deaths : (float)playerStats.kills;
    char kdStr[32];
    sprintf_s(kdStr, "%.2f", kd);
    TextRenderer::drawText(kdStr, statsTextX + 0.15f, statsY);

    // Draw information panel (right side)
    float infoPanelX = 0.53f;
    float infoPanelY = -0.15f;
    float infoPanelW = 0.35f;
    float infoPanelH = 0.45f;

    drawPanel(infoPanelX, infoPanelY, infoPanelW, infoPanelH, "INFORMATION");

    // Display info for selected/hovered button
    glColor3f(0.85f, 0.85f, 0.85f);
    std::string desc = buttonDescriptions[selectedOption];
    size_t pipePos = desc.find('|');

    if (pipePos != std::string::npos) {
        std::string line1 = desc.substr(0, pipePos);
        std::string line2 = desc.substr(pipePos + 1);

        float infoTextX = infoPanelX + 0.02f;
        TextRenderer::drawText(line1, infoTextX, infoPanelY + 0.35f);
        TextRenderer::drawText(line2, infoTextX, infoPanelY + 0.25f);
    }

    // Draw footer with controls and version info
    glColor3f(0.5f, 0.5f, 0.5f);
    TextRenderer::drawText("v1.0.0 Alpha", -0.88f, -0.9f, 0.9f);

    glColor3f(0.6f, 0.6f, 0.7f);
    TextRenderer::drawText("Click buttons or use Arrow Keys + Enter", 0.15f, -0.9f, 0.9f);
}

void MainMenuUI::handleInput(char key) {
    // Up arrow or W
    if (key == 'w' || key == 'W' || key == -32) {
        selectedOption = (selectedOption - 1 + 4) % 4;
    }

    // Down arrow or S
    if (key == 's' || key == 'S') {
        selectedOption = (selectedOption + 1) % 4;
    }

    // Enter or Space
    if (key == '\r' || key == '\n' || key == 13 || key == ' ') {
        selectOption();
    }
}

void MainMenuUI::handleMouseClick(float x, float y) {
    float buttonWidth = 0.5f;
    float buttonHeight = 0.12f;
    float buttonSpacing = 0.02f;
    float centerX = -buttonWidth / 2.0f;
    float startY = 0.4f;

    // Check each button
    for (int i = 0; i < 4; i++) {
        float btnY = startY - i * (buttonHeight + buttonSpacing);

        if (isPointInRect(x, y, centerX, btnY, buttonWidth, buttonHeight)) {
            selectedOption = i;
            selectOption();
            return;
        }
    }
}

void MainMenuUI::selectOption() {
    switch (selectedOption) {
        case 0:  // Enter Lobby
            nextState = UIState::LOBBY;
            changeState = true;
            break;

        case 1:  // View Stash
            nextState = UIState::STASH;
            changeState = true;
            break;

        case 2:  // Visit Merchants
            nextState = UIState::MERCHANT;
            changeState = true;
            break;

        case 3:  // Logout
            networkClient->disconnect();
            nextState = UIState::LOGIN;
            changeState = true;
            break;
    }
}

void MainMenuUI::handlePlayerDataResponse(const std::vector<uint8_t>& payload) {
    if (payload.size() < sizeof(PlayerDataResponse)) return;

    PlayerDataResponse resp;
    memcpy(&resp, payload.data(), sizeof(PlayerDataResponse));

    playerStats = resp.stats;
}
