#include "MainMenuUI.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <gl/GL.h>
#include <cstring>
#include <cmath>

void MainMenuUI::update(float deltaTime) {
    // Update animation
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
    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
    glLoadIdentity();

    // Animated title
    float pulse = 0.85f + 0.15f * std::sin(animTime * 2.0f);
    glColor3f(pulse, pulse * 0.85f, pulse * 0.5f);
    TextRenderer::drawTextCentered("EXTRACTION SHOOTER", 0.8f, 2.2f);

    glColor3f(0.65f, 0.65f, 0.65f);
    TextRenderer::drawTextCentered("MAIN MENU", 0.65f, 1.4f);

    // Player stats card (top right)
    float statsX = 0.4f;
    float statsY = 0.35f;
    float statsW = 0.5f;
    float statsH = 0.45f;

    // Shadow
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
    drawBox(statsX + 0.015f, statsY - 0.015f, statsW, statsH, true);

    // Card background
    glColor4f(0.12f, 0.12f, 0.16f, 0.95f);
    drawBox(statsX, statsY, statsW, statsH, true);

    glColor3f(0.4f, 0.35f, 0.25f);
    glLineWidth(2.0f);
    drawBox(statsX, statsY, statsW, statsH, false);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    // Stats title
    glColor3f(0.9f, 0.75f, 0.5f);
    TextRenderer::drawText("PLAYER STATS", statsX + 0.03f, statsY + statsH - 0.06f, 1.1f);

    // Stats content
    float statY = statsY + statsH - 0.15f;
    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawText("Level:", statsX + 0.03f, statY, 0.9f);
    glColor3f(0.5f, 1.0f, 0.5f);
    TextRenderer::drawText(std::to_string(playerStats.level), statsX + 0.25f, statY, 0.9f);

    statY -= 0.08f;
    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawText("Roubles:", statsX + 0.03f, statY, 0.9f);
    glColor3f(1.0f, 0.9f, 0.5f);
    TextRenderer::drawText(std::to_string(playerStats.roubles), statsX + 0.25f, statY, 0.9f);

    statY -= 0.08f;
    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawText("Raids:", statsX + 0.03f, statY, 0.9f);
    glColor3f(0.6f, 0.8f, 1.0f);
    TextRenderer::drawText(std::to_string(playerStats.raidsCompleted), statsX + 0.25f, statY, 0.9f);

    statY -= 0.08f;
    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawText("K/D:", statsX + 0.03f, statY, 0.9f);
    float kd = playerStats.deaths > 0 ? (float)playerStats.kills / playerStats.deaths : (float)playerStats.kills;
    char kdStr[32];
    sprintf_s(kdStr, "%.2f", kd);
    glColor3f(1.0f, 0.7f, 0.7f);
    TextRenderer::drawText(kdStr, statsX + 0.25f, statY, 0.9f);

    // Button grid (2x2) - properly spaced and centered
    float btnW = 0.35f;
    float btnH = 0.13f;
    float gridGap = 0.03f;  // Gap between buttons
    float totalW = btnW * 2 + gridGap;
    float totalH = btnH * 2 + gridGap;
    float gridX = -totalW / 2.0f;
    float gridY = 0.1f;

    // Button data
    struct ButtonData {
        std::string text;
        int index;
        float x, y;
    };

    ButtonData buttons[4] = {
        {"ENTER LOBBY", 0, gridX, gridY},
        {"VIEW STASH", 1, gridX + btnW + gridGap, gridY},
        {"MERCHANTS", 2, gridX, gridY - btnH - gridGap},
        {"LOGOUT", 3, gridX + btnW + gridGap, gridY - btnH - gridGap}
    };

    // Draw all buttons
    for (int i = 0; i < 4; i++) {
        ButtonData& btn = buttons[i];
        bool isHover = isPointInRect(mouseX, mouseY, btn.x, btn.y, btnW, btnH);

        // Shadow
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
        drawBox(btn.x + 0.01f, btn.y - 0.01f, btnW, btnH, true);

        // Button background
        if (isHover) {
            float hoverPulse = 0.3f + 0.1f * std::sin(animTime * 6.0f);
            glColor4f(0.25f + hoverPulse, 0.45f + hoverPulse, 0.75f + hoverPulse, 0.95f);
        } else if (btn.index == selectedOption) {
            glColor4f(0.22f, 0.4f, 0.65f, 0.95f);
        } else {
            glColor4f(0.15f, 0.15f, 0.2f, 0.95f);
        }
        drawBox(btn.x, btn.y, btnW, btnH, true);

        // Button border
        if (isHover) {
            float glowPulse = 0.6f + 0.4f * std::sin(animTime * 8.0f);
            glColor3f(glowPulse, glowPulse * 0.85f, 1.0f);
            glLineWidth(3.0f);
        } else {
            glColor3f(0.4f, 0.4f, 0.5f);
            glLineWidth(2.0f);
        }
        drawBox(btn.x, btn.y, btnW, btnH, false);
        glLineWidth(1.0f);
        glDisable(GL_BLEND);

        // Button text - properly centered
        glColor3f(1.0f, 1.0f, 1.0f);
        float textX = btn.x + btnW / 2.0f - btn.text.length() * 0.0075f;
        float textY = btn.y + btnH / 2.0f - 0.02f;
        TextRenderer::drawText(btn.text, textX, textY, 1.0f);

        // Update selected option on hover
        if (isHover) {
            selectedOption = btn.index;
        }
    }

    // Information panel (bottom)
    float infoX = -0.7f;
    float infoY = -0.7f;
    float infoW = 1.4f;
    float infoH = 0.25f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f);
    drawBox(infoX + 0.01f, infoY - 0.01f, infoW, infoH, true);

    glColor4f(0.1f, 0.1f, 0.12f, 0.9f);
    drawBox(infoX, infoY, infoW, infoH, true);

    glColor3f(0.35f, 0.35f, 0.4f);
    glLineWidth(1.5f);
    drawBox(infoX, infoY, infoW, infoH, false);
    glLineWidth(1.0f);
    glDisable(GL_BLEND);

    // Info content based on selection
    glColor3f(0.85f, 0.85f, 0.85f);
    const char* infoTexts[4][2] = {
        {"ENTER LOBBY", "Create or join a party to enter raids. Queue with friends for cooperative gameplay."},
        {"VIEW STASH", "Manage your persistent inventory. Store weapons, armor, and loot between raids."},
        {"MERCHANTS", "Trade with merchants. Buy gear, sell loot, and complete quests for better prices."},
        {"LOGOUT", "Disconnect and return to login screen. Your progress will be saved automatically."}
    };

    TextRenderer::drawText(infoTexts[selectedOption][0], infoX + 0.03f, infoY + infoH - 0.07f, 1.1f);
    glColor3f(0.7f, 0.7f, 0.7f);
    TextRenderer::drawText(infoTexts[selectedOption][1], infoX + 0.03f, infoY + infoH - 0.15f, 0.85f);

    // Footer
    glColor3f(0.45f, 0.45f, 0.45f);
    TextRenderer::drawText("v1.0.0 Alpha", -0.88f, -0.92f, 0.7f);
    TextRenderer::drawText("Click any button to continue", 0.3f, -0.92f, 0.7f);
}

void MainMenuUI::handleInput(char key) {
    // Up/Down - cycle options
    if (key == 'w' || key == 'W' || key == 72) {  // W or Up arrow
        selectedOption = (selectedOption - 1 + 4) % 4;
    }
    if (key == 's' || key == 'S' || key == 80) {  // S or Down arrow
        selectedOption = (selectedOption + 1) % 4;
    }

    // Enter - select
    if (key == '\r' || key == '\n' || key == 13) {
        selectOption();
    }

    // Number keys 1-4
    if (key >= '1' && key <= '4') {
        selectedOption = key - '1';
        selectOption();
    }
}

void MainMenuUI::handleMouseClick(float x, float y) {
    // Button dimensions
    float btnW = 0.35f;
    float btnH = 0.13f;
    float gridGap = 0.03f;
    float totalW = btnW * 2 + gridGap;
    float gridX = -totalW / 2.0f;
    float gridY = 0.1f;

    // Check all 4 buttons
    float positions[4][2] = {
        {gridX, gridY},
        {gridX + btnW + gridGap, gridY},
        {gridX, gridY - btnH - gridGap},
        {gridX + btnW + gridGap, gridY - btnH - gridGap}
    };

    for (int i = 0; i < 4; i++) {
        if (isPointInRect(x, y, positions[i][0], positions[i][1], btnW, btnH)) {
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

        case 2:  // Merchants
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
