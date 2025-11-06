#include "MainMenuUI.h"

void MainMenuUI::update(float deltaTime) {
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

    // Draw title
    glColor3f(1.0f, 1.0f, 1.0f);
    TextRenderer::drawTextCentered("EXTRACTION SHOOTER - MAIN MENU", 0.9f, 1.5f);

    // Draw player stats panel
    drawPanel(-0.7f, 0.4f, 0.6f, 0.4f, "Player Stats");

    glColor3f(1.0f, 1.0f, 1.0f);
    TextRenderer::drawText("Level: " + std::to_string(playerStats.level), -0.65f, 0.65f);
    TextRenderer::drawText("Roubles: " + std::to_string(playerStats.roubles), -0.65f, 0.55f);
    TextRenderer::drawText("Raids: " + std::to_string(playerStats.raidsCompleted), -0.65f, 0.45f);

    // Draw menu options panel
    drawPanel(-0.05f, 0.4f, 0.6f, 0.4f, "Menu");

    std::vector<std::string> options = {
        "1. View Stash",
        "2. Visit Merchants",
        "3. Enter Lobby",
        "4. Logout"
    };

    float yPos = 0.65f;
    for (size_t i = 0; i < options.size(); i++) {
        if ((int)i == selectedOption) {
            glColor3f(0.3f, 1.0f, 0.3f);  // Highlight selected
            TextRenderer::drawText("> " + options[i], 0.0f, yPos);
        } else {
            glColor3f(0.8f, 0.8f, 0.8f);
            TextRenderer::drawText("  " + options[i], 0.0f, yPos);
        }
        yPos -= 0.1f;
    }

    // Draw info panel
    drawPanel(-0.7f, -0.3f, 1.3f, 0.5f, "Information");

    glColor3f(0.8f, 0.8f, 0.8f);
    if (selectedOption == 0) {
        TextRenderer::drawText("Stash - Manage your persistent inventory", -0.65f, 0.0f);
        TextRenderer::drawText("Store weapons, armor, loot, and other items here", -0.65f, -0.1f);
    } else if (selectedOption == 1) {
        TextRenderer::drawText("Merchants - Buy and sell items", -0.65f, 0.0f);
        TextRenderer::drawText("5 merchants available: Fence, Prapor, Therapist, Peacekeeper, Ragman", -0.65f, -0.1f);
    } else if (selectedOption == 2) {
        TextRenderer::drawText("Lobby - Create or join a party to enter raids", -0.65f, 0.0f);
        TextRenderer::drawText("Queue with friends for cooperative extraction gameplay", -0.65f, -0.1f);
    } else if (selectedOption == 3) {
        TextRenderer::drawText("Logout - Return to login screen", -0.65f, 0.0f);
    }

    // Draw controls
    glColor3f(0.6f, 0.6f, 0.6f);
    TextRenderer::drawText("Controls: UP/DOWN - Select | ENTER - Confirm", -0.9f, -0.9f, 0.8f);
}

void MainMenuUI::handleInput(char key) {
    // Up arrow or W
    if (key == 'w' || key == 'W' || key == -32) {  // -32 is arrow key prefix on Windows
        selectedOption = (selectedOption - 1 + 4) % 4;
    }

    // Down arrow or S
    if (key == 's' || key == 'S') {
        selectedOption = (selectedOption + 1) % 4;
    }

    // Enter or number keys
    if (key == '\r' || key == '\n' || key == 13) {
        selectOption();
    } else if (key >= '1' && key <= '4') {
        selectedOption = key - '1';
        selectOption();
    }
}

void MainMenuUI::selectOption() {
    switch (selectedOption) {
        case 0:  // View Stash
            nextState = UIState::STASH;
            changeState = true;
            break;

        case 1:  // Visit Merchants
            nextState = UIState::MERCHANT;
            changeState = true;
            break;

        case 2:  // Enter Lobby
            nextState = UIState::LOBBY;
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
