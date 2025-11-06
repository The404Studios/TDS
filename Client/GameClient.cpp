#include "GameClient.h"
#include <iostream>
#include <cstring>

// Manual gluPerspective implementation (GLU might not be available)
static void myPerspective(double fovy, double aspect, double zNear, double zFar) {
    double fH = std::tan(fovy / 360.0 * 3.14159265) * zNear;
    double fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

void GameClient::update(float deltaTime) {
    // Process server packets
    while (networkClient->hasPackets()) {
        auto packet = networkClient->getNextPacket();

        switch (packet.type) {
            case PacketType::SPAWN_INFO:
                handleSpawnInfo(packet.payload);
                break;

            case PacketType::PLAYER_DAMAGE:
                handlePlayerDamage(packet.payload);
                break;

            case PacketType::PLAYER_DEATH:
                handlePlayerDeath(packet.payload);
                break;

            case PacketType::EXTRACTION_COMPLETE:
                handleExtractionComplete(packet.payload);
                break;

            default:
                break;
        }
    }

    // Send position updates (every 100ms)
    static float syncTimer = 0;
    syncTimer += deltaTime;
    if (syncTimer >= 0.1f && alive) {
        sendPositionUpdate();
        syncTimer = 0;
    }
}

void GameClient::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (!alive) {
        renderDeathScreen();
        return;
    }

    if (extracted) {
        renderExtractedScreen();
        return;
    }

    // Render simple 3D view
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    myPerspective(90.0, 1.0, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera rotation
    glRotatef(-playerPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-playerYaw, 0.0f, 1.0f, 0.0f);
    glTranslatef(-playerX, -playerY, -playerZ);

    // Render simple ground
    renderGround();

    // Switch to 2D for HUD
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Render HUD
    renderHUD();
}

void GameClient::handleInput(char key) {
    if (!alive || extracted) {
        // Press any key to return to lobby
        if (key != 0) {
            nextState = UIState::LOBBY;
            changeState = true;
        }
        return;
    }

    float moveSpeed = 0.2f;
    float turnSpeed = 5.0f;

    // Movement
    if (key == 'w' || key == 'W') {
        float rad = playerYaw * 3.14159f / 180.0f;
        playerX += sin(rad) * moveSpeed;
        playerZ += cos(rad) * moveSpeed;
    }
    if (key == 's' || key == 'S') {
        float rad = playerYaw * 3.14159f / 180.0f;
        playerX -= sin(rad) * moveSpeed;
        playerZ -= cos(rad) * moveSpeed;
    }
    if (key == 'a' || key == 'A') {
        playerYaw -= turnSpeed;
    }
    if (key == 'd' || key == 'D') {
        playerYaw += turnSpeed;
    }

    // Extract (test)
    if (key == 'e' || key == 'E') {
        // Check if near extraction zone (simplified)
        if (isNearExtraction()) {
            requestExtraction();
        }
    }

    // Exit to lobby
    if (key == 27) {  // ESC
        nextState = UIState::LOBBY;
        changeState = true;
    }
}

void GameClient::renderGround() {
    glColor3f(0.3f, 0.4f, 0.3f);  // Green-ish ground
    glBegin(GL_QUADS);
    glVertex3f(-500, 0, -500);
    glVertex3f(500, 0, -500);
    glVertex3f(500, 0, 500);
    glVertex3f(-500, 0, 500);
    glEnd();

    // Grid lines
    glColor3f(0.4f, 0.5f, 0.4f);
    glBegin(GL_LINES);
    for (int i = -50; i <= 50; i += 5) {
        glVertex3f(i * 10.0f, 0.01f, -500);
        glVertex3f(i * 10.0f, 0.01f, 500);
        glVertex3f(-500, 0.01f, i * 10.0f);
        glVertex3f(500, 0.01f, i * 10.0f);
    }
    glEnd();
}

void GameClient::renderHUD() {
    // Health bar
    glColor3f(0.8f, 0.0f, 0.0f);  // Red background
    drawBox(-0.95f, -0.95f, 0.4f, 0.05f, true);

    glColor3f(0.0f, 1.0f, 0.0f);  // Green health
    float healthPercent = health / 440.0f;
    drawBox(-0.95f, -0.95f, 0.4f * healthPercent, 0.05f, true);

    glColor3f(1.0f, 1.0f, 1.0f);
    std::string healthText = "HP: " + std::to_string((int)health) + " / 440";
    TextRenderer::drawText(healthText, -0.93f, -0.93f, 0.8f);

    // Position
    std::string posText = "Pos: (" + std::to_string((int)playerX) + ", " +
                          std::to_string((int)playerY) + ", " + std::to_string((int)playerZ) + ")";
    TextRenderer::drawText(posText, -0.95f, 0.9f, 0.8f);

    // Crosshair
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(-0.02f, 0.0f);
    glVertex2f(0.02f, 0.0f);
    glVertex2f(0.0f, -0.02f);
    glVertex2f(0.0f, 0.02f);
    glEnd();

    // Controls
    glColor3f(0.6f, 0.6f, 0.6f);
    TextRenderer::drawText("WASD - Move | E - Extract | ESC - Exit", -0.95f, -0.85f, 0.7f);

    // Extraction prompt
    if (isNearExtraction()) {
        glColor3f(0.3f, 1.0f, 0.3f);
        TextRenderer::drawTextCentered("Press E to Extract", 0.7f, 1.2f);
    }
}

void GameClient::renderDeathScreen() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(1.0f, 0.0f, 0.0f);
    TextRenderer::drawTextCentered("YOU DIED", 0.2f, 2.0f);

    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawTextCentered("You have lost all your gear", 0.0f, 1.2f);
    TextRenderer::drawTextCentered("Press any key to return to lobby", -0.3f, 1.0f);
}

void GameClient::renderExtractedScreen() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(0.0f, 1.0f, 0.0f);
    TextRenderer::drawTextCentered("EXTRACTED", 0.2f, 2.0f);

    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawTextCentered("You have successfully extracted!", 0.0f, 1.2f);
    TextRenderer::drawTextCentered("Your loot has been transferred to your stash", -0.2f, 1.0f);
    TextRenderer::drawTextCentered("Press any key to return to lobby", -0.4f, 0.8f);
}

void GameClient::sendPositionUpdate() {
    PlayerMove move;
    move.x = playerX;
    move.y = playerY;
    move.z = playerZ;
    move.yaw = playerYaw;
    move.pitch = playerPitch;
    move.movementFlags = 0;

    networkClient->sendPacket(PacketType::PLAYER_MOVE, &move, sizeof(move));
}

void GameClient::handleSpawnInfo(const std::vector<uint8_t>& payload) {
    if (payload.size() < sizeof(SpawnInfo)) return;

    SpawnInfo spawn;
    memcpy(&spawn, payload.data(), sizeof(SpawnInfo));

    playerX = spawn.spawnX;
    playerY = spawn.spawnY;
    playerZ = spawn.spawnZ;
    playerYaw = spawn.spawnYaw;

    std::cout << "[GameClient] Spawned at (" << playerX << ", " << playerY << ", " << playerZ << ")" << std::endl;
}

void GameClient::handlePlayerDamage(const std::vector<uint8_t>& payload) {
    if (payload.size() < sizeof(PlayerDamage)) return;

    PlayerDamage damage;
    memcpy(&damage, payload.data(), sizeof(PlayerDamage));

    if (damage.targetAccountId == accountId) {
        health -= damage.damage;
        if (health < 0) health = 0;

        std::cout << "[GameClient] Took " << damage.damage << " damage! HP: " << health << std::endl;
    }
}

void GameClient::handlePlayerDeath(const std::vector<uint8_t>& payload) {
    if (payload.size() < sizeof(PlayerDeath)) return;

    PlayerDeath death;
    memcpy(&death, payload.data(), sizeof(PlayerDeath));

    if (death.victimAccountId == accountId) {
        alive = false;
        std::cout << "[GameClient] Player died!" << std::endl;
    }
}

void GameClient::handleExtractionComplete(const std::vector<uint8_t>& payload) {
    if (payload.size() < sizeof(ExtractionComplete)) return;

    ExtractionComplete extraction;
    memcpy(&extraction, payload.data(), sizeof(ExtractionComplete));

    if (extraction.extracted) {
        extracted = true;
        std::cout << "[GameClient] Extraction successful! Gained " << extraction.roubles << " roubles" << std::endl;
    }
}

bool GameClient::isNearExtraction() {
    // Simplified: Check if near (0, 0) extraction point
    float dist = sqrt(playerX * playerX + playerZ * playerZ);
    return dist < 10.0f;
}

void GameClient::requestExtraction() {
    // Send extraction request
    PlayerMove extract;  // Reusing struct for simplicity
    extract.x = playerX;
    extract.y = playerY;
    extract.z = playerZ;

    // In production, send proper extraction packet
    std::cout << "[GameClient] Requesting extraction..." << std::endl;
}
