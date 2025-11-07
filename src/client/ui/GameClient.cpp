#include "GameClient.h"
#include "../../engine/core/Platform.h"
#include <iostream>
#include <cstring>
#include <algorithm>

// Manual gluPerspective implementation
static void myPerspective(double fovy, double aspect, double zNear, double zFar) {
    double fH = std::tan(fovy / 360.0 * 3.14159265) * zNear;
    double fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

GameClient::GameClient(NetworkClient* netClient, uint64_t accId)
    : networkClient(netClient), accountId(accId),
      playerX(0), playerY(2.0f), playerZ(0), playerYaw(0), playerPitch(0),
      leanAngle(0), isLeaning(false), alive(true), extracted(false),
      mouseCaptured(true), mouseSensitivity(0.2f),
      showInventory(false), inventoryAnimProgress(0.0f), selectedSlot(0),
      showMagCheck(false), magCheckTimer(0.0f), currentAmmo(30), reserveAmmo(120),
      terrainSize(200), terrainScale(2.0f),
      timeOfDay(12.0f), sunAngle(0.0f), rng(std::random_device{}())
{
    // Initialize inventory with starter gear
    ClientInventoryItem weapon;
    weapon.type = ItemType::WEAPON;
    weapon.count = 1;
    weapon.maxStack = 1;
    inventory.push_back(weapon);

    ClientInventoryItem armor;
    armor.type = ItemType::ARMOR;
    armor.count = 1;
    armor.maxStack = 1;
    inventory.push_back(armor);

    ClientInventoryItem medical;
    medical.type = ItemType::MEDICAL;
    medical.count = 2;
    medical.maxStack = 5;
    inventory.push_back(medical);

    ClientInventoryItem ammo;
    ammo.type = ItemType::AMMO;
    ammo.count = 120;
    ammo.maxStack = 200;
    inventory.push_back(ammo);

    // Generate world
    generateTerrain();
    generateTrees();
    generateHouses();
    generateLoot();
    generateExtractionPoints();
    generateEnemies();

    std::cout << "[GameClient] World generated - Terrain: " << terrainSize << "x" << terrainSize
              << ", Trees: " << trees.size() << ", Houses: " << houses.size()
              << ", Loot: " << lootSpawns.size() << ", Enemies: " << enemies.size() << std::endl;
}

void GameClient::update(float deltaTime) {
    if (!alive || extracted) return;

    // Update camera and lean
    updateCamera(deltaTime);
    updateLean(deltaTime);
    updateInventoryAnimation(deltaTime);
    updateEnemies(deltaTime);
    updateTimeOfDay(deltaTime);

    // Update sun angle based on time
    sunAngle = (timeOfDay / 24.0f) * 360.0f;

    // Check nearby interactions
    checkLootPickup();

    // Magazine check timer
    if (showMagCheck) {
        magCheckTimer += deltaTime;
        if (magCheckTimer > 3.0f) {
            showMagCheck = false;
        }
    }

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

    // Send position updates
    static float syncTimer = 0;
    syncTimer += deltaTime;
    if (syncTimer >= 0.1f) {
        sendPositionUpdate();
        syncTimer = 0;
    }
}

void GameClient::render() {
    if (!alive) {
        renderDeathScreen();
        return;
    }

    if (extracted) {
        renderExtractedScreen();
        return;
    }

    // Render 3D world
    render3DWorld();

    // Render 2D HUD
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    renderHUD();
    renderHealthLimbSystem();
    renderCrosshair();
    renderCompass();

    if (showInventory) {
        renderInventory();
    }

    if (showMagCheck) {
        renderMagCheck();
    }
    glEnable(GL_DEPTH_TEST);
}

void GameClient::handleInput(char key) {
    if (!alive || extracted) {
        if (key != 0) {
            nextState = UIState::MAIN_MENU;
            changeState = true;
        }
        return;
    }

    float moveSpeed = 5.0f;
    float rad = playerYaw * 3.14159f / 180.0f;

    // Movement - WASD
    if (key == 'w' || key == 'W') {
        playerX += sin(rad) * moveSpeed * 0.016f;
        playerZ += cos(rad) * moveSpeed * 0.016f;
    }
    if (key == 's' || key == 'S') {
        playerX -= sin(rad) * moveSpeed * 0.016f;
        playerZ -= cos(rad) * moveSpeed * 0.016f;
    }
    if (key == 'a' || key == 'A') {
        playerX -= cos(rad) * moveSpeed * 0.016f;
        playerZ += sin(rad) * moveSpeed * 0.016f;
    }
    if (key == 'd' || key == 'D') {
        playerX += cos(rad) * moveSpeed * 0.016f;
        playerZ -= sin(rad) * moveSpeed * 0.016f;
    }

    // Leaning - Q/E
    if (key == 'q' || key == 'Q') {
        isLeaning = true;
        leanAngle = -15.0f;  // Lean left
    }
    if (key == 'e' || key == 'E') {
        // Check if near extraction first
        if (isNearExtraction()) {
            requestExtraction();
        } else {
            isLeaning = true;
            leanAngle = 15.0f;  // Lean right
        }
    }

    // Inventory - TAB
    if (key == 9) {  // TAB
        showInventory = !showInventory;
    }

    // Magazine check - T (with Alt modifier handled separately)
    if (key == 't' || key == 'T') {
        if (GetAsyncKeyState(VK_MENU) & 0x8000) {  // Alt+T
            showMagCheck = true;
            magCheckTimer = 0.0f;
        }
    }

    // Interaction - F
    if (key == 'f' || key == 'F') {
        checkLootPickup();
    }

    // Exit - ESC
    if (key == 27) {
        nextState = UIState::MAIN_MENU;
        changeState = true;
    }

    // Update player Y based on terrain
    playerY = getTerrainHeight(playerX, playerZ) + 1.7f;
}

void GameClient::handleMouseMove(float deltaX, float deltaY) {
    if (!mouseCaptured || !alive) return;

    playerYaw += deltaX * mouseSensitivity;
    playerPitch -= deltaY * mouseSensitivity;

    // Clamp pitch
    if (playerPitch > 89.0f) playerPitch = 89.0f;
    if (playerPitch < -89.0f) playerPitch = -89.0f;
}

void GameClient::handleMouseClick(float x, float y) {
    if (!alive || showInventory) return;

    // Fire weapon (simplified)
    if (currentAmmo > 0) {
        currentAmmo--;
        std::cout << "[GameClient] Fired! Ammo: " << currentAmmo << "/" << reserveAmmo << std::endl;
    }
}

// ===== TERRAIN GENERATION =====
void GameClient::generateTerrain() {
    terrainHeights.resize(terrainSize);
    for (int i = 0; i < terrainSize; i++) {
        terrainHeights[i].resize(terrainSize);
    }

    // Simple Perlin-like noise generation
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int x = 0; x < terrainSize; x++) {
        for (int z = 0; z < terrainSize; z++) {
            float height = 0;
            float freq = 1.0f;
            float amp = 1.0f;

            // Multi-octave noise
            for (int octave = 0; octave < 4; octave++) {
                float nx = x * freq / terrainSize;
                float nz = z * freq / terrainSize;

                // Simple noise
                float noise = sin(nx * 10.0f) * cos(nz * 10.0f) + dist(rng) * 0.5f;
                height += noise * amp;

                freq *= 2.0f;
                amp *= 0.5f;
            }

            terrainHeights[x][z] = height * 5.0f;  // Scale height
        }
    }
}

void GameClient::generateTrees() {
    std::uniform_real_distribution<float> posDist(-terrainSize / 2.0f * terrainScale, terrainSize / 2.0f * terrainScale);
    std::uniform_real_distribution<float> heightDist(8.0f, 15.0f);
    std::uniform_real_distribution<float> radiusDist(1.0f, 2.0f);

    for (int i = 0; i < 500; i++) {  // 500 trees
        ClientTree tree;
        tree.x = posDist(rng);
        tree.z = posDist(rng);
        tree.height = heightDist(rng);
        tree.radius = radiusDist(rng);
        trees.push_back(tree);
    }
}

void GameClient::generateHouses() {
    std::uniform_real_distribution<float> posDist(-150.0f, 150.0f);

    for (int i = 0; i < 10; i++) {  // 10 houses
        ClientHouse house;
        house.x = posDist(rng);
        house.z = posDist(rng);
        house.width = 15.0f;
        house.height = 8.0f;
        house.depth = 12.0f;
        house.hasLoot = true;

        // Add loot inside house
        for (int j = 0; j < 3; j++) {
            ClientLootSpawn loot;
            loot.x = house.x + (rand() % 10 - 5);
            loot.z = house.z + (rand() % 8 - 4);
            loot.y = getTerrainHeight(loot.x, loot.z) + 0.5f;
            loot.itemType = static_cast<ItemType>(rand() % 10);
            loot.collected = false;
            house.loot.push_back(loot);
        }

        houses.push_back(house);
    }
}

void GameClient::generateLoot() {
    std::uniform_real_distribution<float> posDist(-200.0f, 200.0f);

    for (int i = 0; i < 100; i++) {  // 100 random loot spawns
        ClientLootSpawn loot;
        loot.x = posDist(rng);
        loot.z = posDist(rng);
        loot.y = getTerrainHeight(loot.x, loot.z) + 0.5f;
        loot.itemType = static_cast<ItemType>(rand() % 20);
        loot.collected = false;
        lootSpawns.push_back(loot);
    }
}

void GameClient::generateExtractionPoints() {
    ClientExtractionPoint nw = {-180.0f, -180.0f, 15.0f, "Northwest Extract"};
    ClientExtractionPoint se = {180.0f, 180.0f, 15.0f, "Southeast Extract"};
    ClientExtractionPoint sw = {-180.0f, 180.0f, 15.0f, "Southwest Extract"};
    ClientExtractionPoint ne = {180.0f, -180.0f, 15.0f, "Northeast Extract"};

    extractionPoints.push_back(nw);
    extractionPoints.push_back(se);
    extractionPoints.push_back(sw);
    extractionPoints.push_back(ne);
}

void GameClient::generateEnemies() {
    std::uniform_real_distribution<float> posDist(-150.0f, 150.0f);

    for (int i = 0; i < 15; i++) {  // 15 enemies
        ClientEnemy enemy;
        enemy.id = i;
        enemy.x = posDist(rng);
        enemy.z = posDist(rng);
        enemy.y = getTerrainHeight(enemy.x, enemy.z) + 1.7f;
        enemy.yaw = static_cast<float>(rand() % 360);
        enemy.health = 100.0f;
        enemy.alive = true;
        enemy.lastSeenPlayer = -1.0f;
        enemy.patrolAngle = 0.0f;
        enemies.push_back(enemy);
    }
}

// ===== 3D RENDERING =====
void GameClient::render3DWorld() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    myPerspective(90.0, 16.0 / 9.0, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Apply lighting
    applyLighting();

    // Camera with lean
    glRotatef(-playerPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-playerYaw, 0.0f, 1.0f, 0.0f);
    glRotatef(leanAngle, 0.0f, 0.0f, 1.0f);  // Lean effect
    glTranslatef(-playerX, -playerY, -playerZ);

    renderSkybox();
    renderTerrain();
    renderTrees();
    renderHouses();
    renderLoot();
    renderExtractionPoints();
    renderEnemies();
    renderOtherPlayers();
}

void GameClient::renderSkybox() {
    glDisable(GL_DEPTH_TEST);

    // Sky color based on time of day
    float dayProgress = timeOfDay / 24.0f;
    float r, g, b;

    if (timeOfDay >= 6.0f && timeOfDay < 18.0f) {
        // Day time - light blue
        r = 0.53f;
        g = 0.81f;
        b = 0.92f;
    } else {
        // Night time - dark blue
        r = 0.05f;
        g = 0.05f;
        b = 0.15f;
    }

    glClearColor(r, g, b, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void GameClient::applyLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // Sun light
    float lightDir[] = {sin(sunAngle * 3.14159f / 180.0f), cos(sunAngle * 3.14159f / 180.0f), 0.0f, 0.0f};
    float lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    float lightDiffuse[] = {0.8f, 0.8f, 0.7f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
}

void GameClient::renderTerrain() {
    glColor3f(0.3f, 0.5f, 0.2f);  // Green grass

    for (int x = 0; x < terrainSize - 1; x++) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int z = 0; z < terrainSize; z++) {
            float x1 = (x - terrainSize / 2) * terrainScale;
            float z1 = (z - terrainSize / 2) * terrainScale;
            float x2 = (x + 1 - terrainSize / 2) * terrainScale;

            glVertex3f(x1, terrainHeights[x][z], z1);
            glVertex3f(x2, terrainHeights[x + 1][z], z1);
        }
        glEnd();
    }
}

void GameClient::renderTrees() {
    for (const auto& tree : trees) {
        float y = getTerrainHeight(tree.x, tree.z);

        // Tree trunk (brown cylinder)
        glColor3f(0.4f, 0.25f, 0.1f);
        glBegin(GL_QUADS);
        glVertex3f(tree.x - tree.radius * 0.3f, y, tree.z);
        glVertex3f(tree.x + tree.radius * 0.3f, y, tree.z);
        glVertex3f(tree.x + tree.radius * 0.3f, y + tree.height * 0.7f, tree.z);
        glVertex3f(tree.x - tree.radius * 0.3f, y + tree.height * 0.7f, tree.z);
        glEnd();

        // Tree foliage (green sphere/cone)
        glColor3f(0.1f, 0.6f, 0.1f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(tree.x, y + tree.height, tree.z);  // Top
        for (int i = 0; i <= 8; i++) {
            float angle = i * 3.14159f * 2.0f / 8.0f;
            glVertex3f(tree.x + cos(angle) * tree.radius, y + tree.height * 0.6f, tree.z + sin(angle) * tree.radius);
        }
        glEnd();
    }
}

void GameClient::renderHouses() {
    for (const auto& house : houses) {
        float y = getTerrainHeight(house.x, house.z);

        // House walls (brown)
        glColor3f(0.6f, 0.5f, 0.4f);
        glBegin(GL_QUADS);
        // Front
        glVertex3f(house.x - house.width / 2, y, house.z - house.depth / 2);
        glVertex3f(house.x + house.width / 2, y, house.z - house.depth / 2);
        glVertex3f(house.x + house.width / 2, y + house.height, house.z - house.depth / 2);
        glVertex3f(house.x - house.width / 2, y + house.height, house.z - house.depth / 2);
        // Back
        glVertex3f(house.x - house.width / 2, y, house.z + house.depth / 2);
        glVertex3f(house.x + house.width / 2, y, house.z + house.depth / 2);
        glVertex3f(house.x + house.width / 2, y + house.height, house.z + house.depth / 2);
        glVertex3f(house.x - house.width / 2, y + house.height, house.z + house.depth / 2);
        // Left
        glVertex3f(house.x - house.width / 2, y, house.z - house.depth / 2);
        glVertex3f(house.x - house.width / 2, y, house.z + house.depth / 2);
        glVertex3f(house.x - house.width / 2, y + house.height, house.z + house.depth / 2);
        glVertex3f(house.x - house.width / 2, y + house.height, house.z - house.depth / 2);
        // Right
        glVertex3f(house.x + house.width / 2, y, house.z - house.depth / 2);
        glVertex3f(house.x + house.width / 2, y, house.z + house.depth / 2);
        glVertex3f(house.x + house.width / 2, y + house.height, house.z + house.depth / 2);
        glVertex3f(house.x + house.width / 2, y + house.height, house.z - house.depth / 2);
        glEnd();

        // Roof (red/brown)
        glColor3f(0.7f, 0.3f, 0.2f);
        glBegin(GL_TRIANGLES);
        // Front slope
        glVertex3f(house.x, y + house.height + 3.0f, house.z);
        glVertex3f(house.x - house.width / 2, y + house.height, house.z - house.depth / 2);
        glVertex3f(house.x + house.width / 2, y + house.height, house.z - house.depth / 2);
        // Back slope
        glVertex3f(house.x, y + house.height + 3.0f, house.z);
        glVertex3f(house.x - house.width / 2, y + house.height, house.z + house.depth / 2);
        glVertex3f(house.x + house.width / 2, y + house.height, house.z + house.depth / 2);
        glEnd();
    }
}

void GameClient::renderLoot() {
    for (const auto& loot : lootSpawns) {
        if (loot.collected) continue;

        // Draw as small cube
        glColor3f(1.0f, 0.8f, 0.0f);  // Golden color
        float size = 0.5f;
        glBegin(GL_QUADS);
        // Simple cube (6 faces)
        glVertex3f(loot.x - size, loot.y - size, loot.z - size);
        glVertex3f(loot.x + size, loot.y - size, loot.z - size);
        glVertex3f(loot.x + size, loot.y + size, loot.z - size);
        glVertex3f(loot.x - size, loot.y + size, loot.z - size);
        glEnd();
    }
}

void GameClient::renderExtractionPoints() {
    for (const auto& extract : extractionPoints) {
        float y = getTerrainHeight(extract.x, extract.z);

        // Draw extraction marker (green circle)
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 32; i++) {
            float angle = i * 3.14159f * 2.0f / 32.0f;
            glVertex3f(extract.x + cos(angle) * extract.radius, y + 0.1f, extract.z + sin(angle) * extract.radius);
        }
        glEnd();

        // Vertical beam
        glBegin(GL_LINES);
        glVertex3f(extract.x, y, extract.z);
        glVertex3f(extract.x, y + 20.0f, extract.z);
        glEnd();
    }
}

void GameClient::renderEnemies() {
    for (const auto& enemy : enemies) {
        if (!enemy.alive) continue;

        // Draw as simple humanoid shape (red)
        glColor3f(1.0f, 0.0f, 0.0f);

        // Body
        glBegin(GL_QUADS);
        glVertex3f(enemy.x - 0.4f, enemy.y - 0.8f, enemy.z);
        glVertex3f(enemy.x + 0.4f, enemy.y - 0.8f, enemy.z);
        glVertex3f(enemy.x + 0.4f, enemy.y + 0.6f, enemy.z);
        glVertex3f(enemy.x - 0.4f, enemy.y + 0.6f, enemy.z);
        glEnd();

        // Head
        glColor3f(1.0f, 0.8f, 0.7f);
        glBegin(GL_QUADS);
        glVertex3f(enemy.x - 0.3f, enemy.y + 0.6f, enemy.z);
        glVertex3f(enemy.x + 0.3f, enemy.y + 0.6f, enemy.z);
        glVertex3f(enemy.x + 0.3f, enemy.y + 1.2f, enemy.z);
        glVertex3f(enemy.x - 0.3f, enemy.y + 1.2f, enemy.z);
        glEnd();
    }
}

void GameClient::renderOtherPlayers() {
    for (const auto& pair : otherPlayers) {
        const OtherPlayer& other = pair.second;
        if (!other.alive) continue;

        // Draw as blue humanoid
        glColor3f(0.0f, 0.5f, 1.0f);

        glBegin(GL_QUADS);
        glVertex3f(other.x - 0.4f, other.y - 0.8f, other.z);
        glVertex3f(other.x + 0.4f, other.y - 0.8f, other.z);
        glVertex3f(other.x + 0.4f, other.y + 0.6f, other.z);
        glVertex3f(other.x - 0.4f, other.y + 0.6f, other.z);
        glEnd();
    }
}

// ===== HUD RENDERING =====
void GameClient::renderHUD() {
    // FPS counter placeholder
    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawText("FPS: 60", 0.7f, 0.9f, 0.7f);

    // Position
    char posBuf[128];
    sprintf_s(posBuf, "Position: (%.1f, %.1f, %.1f)", playerX, playerY, playerZ);
    TextRenderer::drawText(posBuf, -0.95f, 0.9f, 0.7f);

    // Time of day
    int hour = static_cast<int>(timeOfDay);
    int minute = static_cast<int>((timeOfDay - hour) * 60.0f);
    char timeBuf[32];
    sprintf_s(timeBuf, "Time: %02d:%02d", hour, minute);
    TextRenderer::drawText(timeBuf, 0.7f, 0.85f, 0.7f);

    // Ammo counter
    char ammoBuf[64];
    sprintf_s(ammoBuf, "Ammo: %d / %d", currentAmmo, reserveAmmo);
    glColor3f(1.0f, 1.0f, 1.0f);
    TextRenderer::drawText(ammoBuf, 0.65f, -0.9f, 0.9f);

    // Controls hint
    glColor3f(0.6f, 0.6f, 0.6f);
    TextRenderer::drawText("WASD-Move Q/E-Lean TAB-Inv Alt+T-Mag F-Interact", -0.95f, -0.95f, 0.65f);

    // Extraction prompt
    ClientExtractionPoint* nearest = getNearestExtraction();
    if (nearest) {
        float dx = nearest->x - playerX;
        float dz = nearest->z - playerZ;
        float dist = sqrt(dx * dx + dz * dz);

        if (dist < nearest->radius) {
            glColor3f(0.3f, 1.0f, 0.3f);
            std::string extractText = "Press E to extract at " + nearest->name;
            TextRenderer::drawTextCentered(extractText, 0.6f, 1.2f);
        } else if (dist < nearest->radius * 3.0f) {
            glColor3f(0.8f, 0.8f, 0.3f);
            char distBuf[128];
            sprintf_s(distBuf, "%s - %.0fm", nearest->name.c_str(), dist);
            TextRenderer::drawText(distBuf, -0.95f, 0.8f, 0.8f);
        }
    }
}

void GameClient::renderHealthLimbSystem() {
    // Health/Limb UI in top right
    float panelX = 0.55f;
    float panelY = 0.4f;
    float panelW = 0.4f;
    float panelH = 0.55f;

    // Draw panel background
    glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    drawBox(panelX, panelY, panelW, panelH, true);
    glDisable(GL_BLEND);

    // Draw border
    glColor3f(0.5f, 0.5f, 0.5f);
    drawBox(panelX, panelY, panelW, panelH, false);

    // Title
    glColor3f(1.0f, 1.0f, 1.0f);
    TextRenderer::drawText("HEALTH STATUS", panelX + 0.02f, panelY + panelH - 0.05f, 0.8f);

    float yPos = panelY + panelH - 0.1f;
    float barWidth = 0.2f;
    float barHeight = 0.03f;
    float xLabel = panelX + 0.02f;
    float xBar = panelX + 0.15f;

    // Helper lambda to draw limb
    auto drawLimb = [&](const char* name, float health, float maxHealth) {
        glColor3f(0.8f, 0.8f, 0.8f);
        TextRenderer::drawText(name, xLabel, yPos, 0.7f);

        // Health bar background (red)
        glColor3f(0.4f, 0.0f, 0.0f);
        drawBox(xBar, yPos, barWidth, barHeight, true);

        // Health bar foreground (green to red gradient)
        float healthPercent = health / maxHealth;
        float r = 1.0f - healthPercent;
        float g = healthPercent;
        glColor3f(r, g, 0.0f);
        drawBox(xBar, yPos, barWidth * healthPercent, barHeight, true);

        // Health text
        glColor3f(1.0f, 1.0f, 1.0f);
        char healthBuf[32];
        sprintf_s(healthBuf, "%.0f", health);
        TextRenderer::drawText(healthBuf, xBar + barWidth + 0.01f, yPos, 0.7f);

        yPos -= 0.055f;
    };

    drawLimb("Head    ", limbHealth.head, 35.0f);
    drawLimb("Thorax  ", limbHealth.thorax, 80.0f);
    drawLimb("Stomach ", limbHealth.stomach, 70.0f);
    drawLimb("L.Arm   ", limbHealth.leftArm, 60.0f);
    drawLimb("R.Arm   ", limbHealth.rightArm, 60.0f);
    drawLimb("L.Leg   ", limbHealth.leftLeg, 65.0f);
    drawLimb("R.Leg   ", limbHealth.rightLeg, 65.0f);

    // Total health
    float totalHealth = limbHealth.getTotalHealth();
    glColor3f(1.0f, 1.0f, 0.0f);
    char totalBuf[64];
    sprintf_s(totalBuf, "Total: %.0f / 440", totalHealth);
    TextRenderer::drawText(totalBuf, panelX + 0.02f, panelY + 0.02f, 0.9f);
}

void GameClient::renderInventory() {
    // Animated slide-in from right
    float slideX = 1.0f - (inventoryAnimProgress * 0.6f);

    float panelX = slideX;
    float panelY = -0.8f;
    float panelW = 0.55f;
    float panelH = 1.6f;

    // Background
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.05f, 0.05f, 0.1f, 0.95f);
    drawBox(panelX, panelY, panelW, panelH, true);

    glColor3f(0.6f, 0.6f, 0.7f);
    drawBox(panelX, panelY, panelW, panelH, false);

    // Title
    glColor3f(1.0f, 1.0f, 1.0f);
    TextRenderer::drawText("INVENTORY", panelX + 0.02f, panelY + panelH - 0.08f, 1.2f);

    // Items
    float yPos = panelY + panelH - 0.15f;
    for (size_t i = 0; i < inventory.size(); i++) {
        bool selected = (i == static_cast<size_t>(selectedSlot));

        if (selected) {
            glColor4f(0.2f, 0.4f, 0.6f, 0.8f);
            drawBox(panelX + 0.01f, yPos - 0.005f, panelW - 0.02f, 0.06f, true);
        }

        glColor3f(0.9f, 0.9f, 0.9f);
        const char* itemName = ItemDatabase::getItemName(inventory[i].type);
        TextRenderer::drawText(itemName, panelX + 0.03f, yPos, 0.8f);

        // Count
        char countBuf[32];
        sprintf_s(countBuf, "x%d", inventory[i].count);
        TextRenderer::drawText(countBuf, panelX + panelW - 0.1f, yPos, 0.8f);

        yPos -= 0.08f;
    }

    glDisable(GL_BLEND);
}

void GameClient::renderMagCheck() {
    // Magazine check animation in center
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float alpha = 1.0f - (magCheckTimer / 3.0f);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);

    char magBuf[128];
    sprintf_s(magBuf, "Magazine: %d / 30 rounds", currentAmmo);
    TextRenderer::drawTextCentered(magBuf, 0.0f, 1.5f);

    sprintf_s(magBuf, "Reserve: %d rounds", reserveAmmo);
    TextRenderer::drawTextCentered(magBuf, -0.15f, 1.2f);

    glDisable(GL_BLEND);
}

void GameClient::renderCrosshair() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    // Horizontal
    glVertex2f(-0.02f, 0.0f);
    glVertex2f(-0.005f, 0.0f);
    glVertex2f(0.005f, 0.0f);
    glVertex2f(0.02f, 0.0f);
    // Vertical
    glVertex2f(0.0f, -0.02f);
    glVertex2f(0.0f, -0.005f);
    glVertex2f(0.0f, 0.005f);
    glVertex2f(0.0f, 0.02f);
    glEnd();
    glLineWidth(1.0f);
}

void GameClient::renderCompass() {
    // Simple compass at top center
    float compassY = 0.85f;

    glColor3f(0.8f, 0.8f, 0.8f);
    const char* directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
    int dirIndex = static_cast<int>((playerYaw + 22.5f) / 45.0f) % 8;
    TextRenderer::drawTextCentered(directions[dirIndex], compassY, 1.0f);

    char yawBuf[32];
    sprintf_s(yawBuf, "%.0f°", fmod(playerYaw, 360.0f));
    TextRenderer::drawTextCentered(yawBuf, compassY - 0.08f, 0.8f);
}

void GameClient::renderDeathScreen() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.1f, 0.0f, 0.0f, 1.0f);

    glColor3f(1.0f, 0.0f, 0.0f);
    TextRenderer::drawTextCentered("YOU DIED", 0.2f, 2.5f);

    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawTextCentered("You have lost all your gear", 0.0f, 1.2f);
    TextRenderer::drawTextCentered("Press any key to return to main menu", -0.2f, 1.0f);
}

void GameClient::renderExtractedScreen() {
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 0.1f, 0.0f, 1.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    TextRenderer::drawTextCentered("EXTRACTED", 0.2f, 2.5f);

    glColor3f(0.8f, 0.8f, 0.8f);
    TextRenderer::drawTextCentered("You have successfully extracted!", 0.0f, 1.2f);
    TextRenderer::drawTextCentered("Your loot has been transferred to your stash", -0.2f, 1.0f);
    TextRenderer::drawTextCentered("Press any key to return to main menu", -0.4f, 0.8f);
}

// ===== UPDATES =====
void GameClient::updateCamera(float deltaTime) {
    // Camera is updated via mouse movement handled in handleMouseMove
}

void GameClient::updateLean(float deltaTime) {
    if (!isLeaning) {
        // Return to center
        if (leanAngle > 0) {
            leanAngle -= 60.0f * deltaTime;
            if (leanAngle < 0) leanAngle = 0;
        } else if (leanAngle < 0) {
            leanAngle += 60.0f * deltaTime;
            if (leanAngle > 0) leanAngle = 0;
        }
    }

    // Reset lean flag for next frame
    if (isLeaning) {
        isLeaning = false;
    }
}

void GameClient::updateInventoryAnimation(float deltaTime) {
    if (showInventory) {
        inventoryAnimProgress += deltaTime * 5.0f;
        if (inventoryAnimProgress > 1.0f) inventoryAnimProgress = 1.0f;
    } else {
        inventoryAnimProgress -= deltaTime * 5.0f;
        if (inventoryAnimProgress < 0.0f) inventoryAnimProgress = 0.0f;
    }
}

void GameClient::updateEnemies(float deltaTime) {
    for (auto& enemy : enemies) {
        if (!enemy.alive) continue;

        // Simple patrol AI
        enemy.patrolAngle += deltaTime * 20.0f;
        if (enemy.patrolAngle > 360.0f) enemy.patrolAngle -= 360.0f;

        // Move in patrol pattern
        float rad = enemy.patrolAngle * 3.14159f / 180.0f;
        enemy.x += sin(rad) * deltaTime * 0.5f;
        enemy.z += cos(rad) * deltaTime * 0.5f;

        enemy.y = getTerrainHeight(enemy.x, enemy.z) + 1.7f;

        // Check if player is in sight (simplified)
        float dx = playerX - enemy.x;
        float dz = playerZ - enemy.z;
        float dist = sqrt(dx * dx + dz * dz);

        if (dist < 30.0f) {
            // Player detected - face player
            enemy.yaw = atan2(dx, dz) * 180.0f / 3.14159f;
            enemy.lastSeenPlayer = 0.0f;
        }
    }
}

void GameClient::updateTimeOfDay(float deltaTime) {
    timeOfDay += deltaTime * 0.01f;  // Slow time progression
    if (timeOfDay > 24.0f) timeOfDay -= 24.0f;
}

void GameClient::checkLootPickup() {
    for (auto& loot : lootSpawns) {
        if (loot.collected) continue;

        float dx = loot.x - playerX;
        float dz = loot.z - playerZ;
        float dist = sqrt(dx * dx + dz * dz);

        if (dist < 3.0f) {
            // Pickup loot
            loot.collected = true;

            // Add to inventory (simplified)
            bool found = false;
            for (auto& item : inventory) {
                if (item.type == loot.itemType && item.count < item.maxStack) {
                    item.count++;
                    found = true;
                    break;
                }
            }

            if (!found) {
                ClientInventoryItem newItem;
                newItem.type = loot.itemType;
                newItem.count = 1;
                newItem.maxStack = 100;
                inventory.push_back(newItem);
            }

            std::cout << "[GameClient] Picked up loot!" << std::endl;
            break;
        }
    }
}

void GameClient::checkHouseEntry() {
    // Simplified - not implemented yet
}

bool GameClient::checkLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2) {
    // Simplified raycasting - not fully implemented
    return true;
}

float GameClient::getTerrainHeight(float x, float z) {
    int gridX = static_cast<int>((x / terrainScale) + terrainSize / 2);
    int gridZ = static_cast<int>((z / terrainScale) + terrainSize / 2);

    if (gridX < 0 || gridX >= terrainSize || gridZ < 0 || gridZ >= terrainSize) {
        return 0.0f;
    }

    return terrainHeights[gridX][gridZ];
}

bool GameClient::isInsideHouse(float x, float z) {
    for (const auto& house : houses) {
        if (x >= house.x - house.width / 2 && x <= house.x + house.width / 2 &&
            z >= house.z - house.depth / 2 && z <= house.z + house.depth / 2) {
            return true;
        }
    }
    return false;
}

bool GameClient::isNearExtraction() {
    return getNearestExtraction() != nullptr;
}

ClientExtractionPoint* GameClient::getNearestExtraction() {
    for (auto& extract : extractionPoints) {
        float dx = extract.x - playerX;
        float dz = extract.z - playerZ;
        float dist = sqrt(dx * dx + dz * dz);

        if (dist < extract.radius) {
            return &extract;
        }
    }
    return nullptr;
}

// ===== NETWORK =====
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
        // Apply damage to random limb (simplified)
        float* limbs[] = {&limbHealth.head, &limbHealth.thorax, &limbHealth.stomach,
                          &limbHealth.leftArm, &limbHealth.rightArm,
                          &limbHealth.leftLeg, &limbHealth.rightLeg};
        int limbIndex = rand() % 7;
        *limbs[limbIndex] -= damage.damage;

        if (*limbs[limbIndex] < 0) *limbs[limbIndex] = 0;

        std::cout << "[GameClient] Took " << damage.damage << " damage!" << std::endl;

        // Check for death
        if (limbHealth.getTotalHealth() <= 0) {
            alive = false;
        }
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
        std::cout << "[GameClient] Extraction successful!" << std::endl;
    }
}

void GameClient::handleOtherPlayerUpdate(const std::vector<uint8_t>& payload) {
    // Not fully implemented - would sync other players for PvP
}

void GameClient::requestExtraction() {
    std::cout << "[GameClient] Requesting extraction..." << std::endl;

    // Send extraction request (simplified - would need proper packet)
    // For now, just set extracted to true after delay
    extracted = true;
}
