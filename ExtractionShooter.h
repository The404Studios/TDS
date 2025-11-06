#pragma once
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "PlayerCharacter.h"
#include "TerrainManager.h"
#include "InventorySystem.h"
#include "PlayerData.h"
#include <vector>
#include <map>
#include <cmath>
#include <ctime>
#include <cstring>
#include <iostream>

// Loot spawn in the world
struct LootSpawn {
    float x, y, z;
    std::shared_ptr<Item> item;
    bool collected;

    LootSpawn(float x, float y, float z, std::shared_ptr<Item> item)
        : x(x), y(y), z(z), item(item), collected(false) {}
};

// Extraction zone
struct ExtractionZone {
    std::string name;
    float x, z;              // Center position
    float radius;            // Activation radius
    bool active;
    float extractTime;       // Time to extract (seconds)
    float extractProgress;   // Current extraction progress

    ExtractionZone(const std::string& name, float x, float z, float radius, float extractTime)
        : name(name), x(x), z(z), radius(radius), active(true), extractTime(extractTime), extractProgress(0.0f) {}

    bool isPlayerInZone(float px, float pz) {
        float dx = px - x;
        float dz = pz - z;
        float dist = sqrt(dx * dx + dz * dz);
        return dist < radius;
    }
};

// AI enemy (simplified scav)
struct AIEnemy {
    float x, y, z;
    float health;
    float yaw;               // Direction facing
    bool alive;
    std::vector<std::shared_ptr<Item>> loot;  // Items dropped on death

    AIEnemy(float x, float y, float z)
        : x(x), y(y), z(z), health(100.0f), yaw(0.0f), alive(true) {}

    void takeDamage(float damage) {
        health -= damage;
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }
};

// Game states for extraction shooter
enum class GameState {
    MAIN_MENU,
    STASH_SCREEN,
    LOADOUT_SCREEN,
    IN_RAID,
    EXTRACTING,
    POST_RAID,
    DEATH_SCREEN
};

// Main extraction shooter game mode
class ExtractionShooter {
public:
    GameState currentState;
    PlayerData playerData;

    // In-raid components
    PlayerCharacter player;
    TerrainManager terrain;
    std::vector<LootSpawn> lootSpawns;
    std::vector<ExtractionZone> extractions;
    std::vector<AIEnemy> enemies;

    // In-raid state
    std::vector<std::shared_ptr<Item>> collectedLoot;
    float raidTime;          // Time in raid (seconds)
    float maxRaidTime;       // Max raid time (30 minutes)
    ExtractionZone* currentExtraction;

    // Camera controls
    float cameraYaw;
    float cameraPitch;
    float cameraSensitivity;

    // Input state
    bool keys[256];
    int mouseX, mouseY;
    int lastMouseX, lastMouseY;

    // UI state
    bool inventoryOpen;
    std::shared_ptr<Item> selectedItem;  // Item being dragged
    int dragStartX, dragStartY;

    ExtractionShooter()
        : currentState(GameState::MAIN_MENU),
          terrain(512, 512, 2.0f),
          raidTime(0.0f),
          maxRaidTime(30.0f * 60.0f),
          currentExtraction(nullptr),
          cameraYaw(0.0f),
          cameraPitch(0.0f),
          cameraSensitivity(0.2f),
          mouseX(0), mouseY(0),
          lastMouseX(0), lastMouseY(0),
          inventoryOpen(false),
          selectedItem(nullptr),
          dragStartX(0), dragStartY(0) {

        memset(keys, 0, sizeof(keys));

        // Initialize player at spawn point
        player.position = Vector3(256.0f, 0.0f, 256.0f);
        player.health = 440.0f;  // Full health
        player.currentWeapon = WeaponType::RIFLE;

        // Generate terrain
        terrain.generateTerrain();

        // Load or create player data
        if (!playerData.load("playerdata.txt")) {
            // New player - give starting gear
            playerData.giveStartingGear();
            playerData.save("playerdata.txt");
        }
    }

    void startRaid() {
        currentState = GameState::IN_RAID;
        raidTime = 0.0f;
        collectedLoot.clear();
        lootSpawns.clear();
        enemies.clear();

        // Respawn player at starting position
        player.position = Vector3(256.0f, 50.0f, 256.0f);
        player.health = 440.0f;
        player.isDead = false;

        // Set up extraction zones
        extractions.clear();
        extractions.push_back(ExtractionZone("Railroad Bridge", 450.0f, 450.0f, 15.0f, 8.0f));
        extractions.push_back(ExtractionZone("Scav Checkpoint", 50.0f, 50.0f, 15.0f, 10.0f));
        extractions.push_back(ExtractionZone("Boat Dock", 256.0f, 50.0f, 12.0f, 7.0f));

        // Spawn loot around the map
        spawnLoot(50);

        // Spawn AI enemies
        spawnEnemies(10);

        std::cout << "=== RAID STARTED ===\n";
        std::cout << "Extract at: ";
        for (auto& ext : extractions) {
            std::cout << ext.name << ", ";
        }
        std::cout << "\n";
    }

    void update(float deltaTime) {
        switch (currentState) {
            case GameState::MAIN_MENU:
                updateMainMenu(deltaTime);
                break;
            case GameState::STASH_SCREEN:
                updateStashScreen(deltaTime);
                break;
            case GameState::IN_RAID:
                updateInRaid(deltaTime);
                break;
            case GameState::EXTRACTING:
                updateExtracting(deltaTime);
                break;
            case GameState::POST_RAID:
                updatePostRaid(deltaTime);
                break;
            case GameState::DEATH_SCREEN:
                updateDeathScreen(deltaTime);
                break;
        }
    }

    void render() {
        switch (currentState) {
            case GameState::MAIN_MENU:
                renderMainMenu();
                break;
            case GameState::STASH_SCREEN:
                renderStashScreen();
                break;
            case GameState::IN_RAID:
                renderInRaid();
                break;
            case GameState::EXTRACTING:
                renderExtracting();
                break;
            case GameState::POST_RAID:
                renderPostRaid();
                break;
            case GameState::DEATH_SCREEN:
                renderDeathScreen();
                break;
        }
    }

    void handleKeyPress(unsigned char key, bool pressed) {
        if (key < 256) {
            keys[key] = pressed;
        }

        // Handle key-specific actions
        if (pressed) {
            switch (currentState) {
                case GameState::MAIN_MENU:
                    if (key == '1') currentState = GameState::STASH_SCREEN;
                    if (key == '2') startRaid();
                    if (key == 27) exit(0); // ESC to quit
                    break;

                case GameState::STASH_SCREEN:
                    if (key == 27) currentState = GameState::MAIN_MENU; // ESC to go back
                    if (key == 13) startRaid(); // ENTER to start raid
                    break;

                case GameState::IN_RAID:
                    if (key == 27) inventoryOpen = !inventoryOpen; // ESC/TAB to toggle inventory
                    if (key == 'e' || key == 'E') tryPickupLoot();
                    if (key == 'f' || key == 'F') tryExtract();
                    break;

                case GameState::POST_RAID:
                case GameState::DEATH_SCREEN:
                    if (key == 13) {  // ENTER to continue
                        currentState = GameState::MAIN_MENU;
                    }
                    break;
            }
        }
    }

    void handleMouseMove(int x, int y) {
        if (currentState == GameState::IN_RAID && !inventoryOpen) {
            // FPS camera control
            int dx = x - lastMouseX;
            int dy = y - lastMouseY;

            cameraYaw += dx * cameraSensitivity;
            cameraPitch -= dy * cameraSensitivity;

            // Clamp pitch
            if (cameraPitch > 89.0f) cameraPitch = 89.0f;
            if (cameraPitch < -89.0f) cameraPitch = -89.0f;
        }

        lastMouseX = x;
        lastMouseY = y;
        mouseX = x;
        mouseY = y;
    }

private:
    void updateMainMenu(float deltaTime) {
        // Nothing to update
    }

    void updateStashScreen(float deltaTime) {
        // Handle stash management
    }

    void updateInRaid(float deltaTime) {
        if (inventoryOpen) return;  // Paused when inventory is open

        // Update raid timer
        raidTime += deltaTime;

        // Check time limit
        if (raidTime >= maxRaidTime) {
            // Force death - didn't extract in time
            player.health = 0;
            player.isDead = true;
        }

        // Update player movement
        Vector3 movement(0, 0, 0);

        if (keys['w'] || keys['W']) movement.z -= 1.0f;
        if (keys['s'] || keys['S']) movement.z += 1.0f;
        if (keys['a'] || keys['A']) movement.x -= 1.0f;
        if (keys['d'] || keys['D']) movement.x += 1.0f;

        // Transform movement by camera yaw
        float radYaw = cameraYaw * 3.14159f / 180.0f;
        float moveX = movement.x * cos(radYaw) - movement.z * sin(radYaw);
        float moveZ = movement.x * sin(radYaw) + movement.z * cos(radYaw);

        movement.x = moveX;
        movement.z = moveZ;

        // Normalize movement
        float len = sqrt(movement.x * movement.x + movement.z * movement.z);
        if (len > 0.01f) {
            movement.x /= len;
            movement.z /= len;
        }

        // Apply movement
        float speed = (keys[' ']) ? 15.0f : 5.0f;  // Sprint with space
        player.position.x += movement.x * speed * deltaTime;
        player.position.z += movement.z * speed * deltaTime;

        // Snap to terrain
        player.position.y = terrain.getHeightAt(player.position.x, player.position.z) + 1.8f;

        // Update player
        player.update(deltaTime, terrain);

        // Check for extraction zones
        for (auto& ext : extractions) {
            if (ext.active && ext.isPlayerInZone(player.position.x, player.position.z)) {
                currentExtraction = &ext;
                return;
            }
        }
        currentExtraction = nullptr;

        // Check if player died
        if (player.isDead) {
            currentState = GameState::DEATH_SCREEN;
            playerData.handleDeath();
            playerData.save("playerdata.txt");
        }
    }

    void updateExtracting(float deltaTime) {
        if (!currentExtraction) {
            currentState = GameState::IN_RAID;
            return;
        }

        // Increase extraction progress
        currentExtraction->extractProgress += deltaTime;

        // Check if extraction complete
        if (currentExtraction->extractProgress >= currentExtraction->extractTime) {
            // Successfully extracted!
            currentState = GameState::POST_RAID;

            // Add collected loot to player data
            playerData.handleExtraction();

            // Add collected items from raid
            for (auto item : collectedLoot) {
                item->foundInRaid = true;
                playerData.stash.addItem(item);
            }

            playerData.save("playerdata.txt");
        }

        // Check if player left extraction zone
        if (!currentExtraction->isPlayerInZone(player.position.x, player.position.z)) {
            currentExtraction->extractProgress = 0.0f;
            currentExtraction = nullptr;
            currentState = GameState::IN_RAID;
        }
    }

    void updatePostRaid(float deltaTime) {
        // Waiting for player input
    }

    void updateDeathScreen(float deltaTime) {
        // Waiting for player input
    }

    void renderMainMenu() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // Simple 2D menu
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Background
        glColor3f(0.1f, 0.1f, 0.15f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(800, 0);
        glVertex2f(800, 600);
        glVertex2f(0, 600);
        glEnd();

        // Title (draw as colored box for now - text rendering is complex)
        glColor3f(0.8f, 0.6f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(250, 100);
        glVertex2f(550, 100);
        glVertex2f(550, 150);
        glVertex2f(250, 150);
        glEnd();

        // Menu options
        glColor3f(0.3f, 0.3f, 0.4f);
        glBegin(GL_QUADS);
        // Option 1: Stash
        glVertex2f(300, 250);
        glVertex2f(500, 250);
        glVertex2f(500, 300);
        glVertex2f(300, 300);
        // Option 2: Raid
        glVertex2f(300, 320);
        glVertex2f(500, 320);
        glVertex2f(500, 370);
        glVertex2f(300, 370);
        glEnd();

        // Restore
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
    }

    void renderStashScreen() {
        renderMainMenu();  // Simplified - would show stash grid
    }

    void renderInRaid() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set up 3D FPS camera
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(90.0, 800.0 / 600.0, 0.1, 1000.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Calculate camera direction
        float radPitch = cameraPitch * 3.14159f / 180.0f;
        float radYaw = cameraYaw * 3.14159f / 180.0f;

        float lookX = cos(radPitch) * sin(radYaw);
        float lookY = sin(radPitch);
        float lookZ = -cos(radPitch) * cos(radYaw);

        // Position camera at player head
        float eyeHeight = 1.6f;
        gluLookAt(
            player.position.x, player.position.y + eyeHeight, player.position.z,
            player.position.x + lookX, player.position.y + eyeHeight + lookY, player.position.z + lookZ,
            0, 1, 0
        );

        // Render terrain
        terrain.render();

        // Render loot spawns
        renderLootSpawns();

        // Render extraction zones
        renderExtractionZones();

        // Render enemies
        renderEnemies();

        // Render player weapon
        renderPlayerWeapon();

        // Render HUD
        renderHUD();

        if (inventoryOpen) {
            renderInventoryUI();
        }
    }

    void renderExtracting() {
        renderInRaid();  // Same as in raid, but with extraction UI overlay
    }

    void renderPostRaid() {
        renderMainMenu();  // Simplified - would show loot summary
    }

    void renderDeathScreen() {
        renderMainMenu();  // Simplified - would show death screen
    }

    void renderLootSpawns() {
        for (auto& loot : lootSpawns) {
            if (!loot.collected) {
                glPushMatrix();
                glTranslatef(loot.x, loot.y + 0.5f, loot.z);

                // Draw as rotating cube
                glColor3f(1.0f, 0.8f, 0.2f);

                glBegin(GL_QUADS);
                // Front
                glVertex3f(-0.2f, -0.2f, 0.2f);
                glVertex3f(0.2f, -0.2f, 0.2f);
                glVertex3f(0.2f, 0.2f, 0.2f);
                glVertex3f(-0.2f, 0.2f, 0.2f);
                // Add other faces...
                glEnd();

                glPopMatrix();
            }
        }
    }

    void renderExtractionZones() {
        for (auto& ext : extractions) {
            if (ext.active) {
                glPushMatrix();
                glTranslatef(ext.x, terrain.getHeightAt(ext.x, ext.z), ext.z);

                // Draw as green circle on ground
                glColor4f(0.2f, 1.0f, 0.2f, 0.3f);
                glBegin(GL_TRIANGLE_FAN);
                glVertex3f(0, 0.1f, 0);
                for (int i = 0; i <= 20; i++) {
                    float angle = i * 2.0f * 3.14159f / 20.0f;
                    float x = cos(angle) * ext.radius;
                    float z = sin(angle) * ext.radius;
                    glVertex3f(x, 0.1f, z);
                }
                glEnd();

                glPopMatrix();
            }
        }
    }

    void renderEnemies() {
        for (auto& enemy : enemies) {
            if (enemy.alive) {
                glPushMatrix();
                glTranslatef(enemy.x, enemy.y, enemy.z);

                // Draw as red capsule/cylinder
                glColor3f(1.0f, 0.2f, 0.2f);

                // Simple rectangular body
                glBegin(GL_QUADS);
                // Front face
                glVertex3f(-0.2f, 0.0f, 0.2f);
                glVertex3f(0.2f, 0.0f, 0.2f);
                glVertex3f(0.2f, 1.8f, 0.2f);
                glVertex3f(-0.2f, 1.8f, 0.2f);
                // Back face
                glVertex3f(-0.2f, 0.0f, -0.2f);
                glVertex3f(0.2f, 0.0f, -0.2f);
                glVertex3f(0.2f, 1.8f, -0.2f);
                glVertex3f(-0.2f, 1.8f, -0.2f);
                glEnd();

                glPopMatrix();
            }
        }
    }

    void renderPlayerWeapon() {
        // Draw simple weapon model in bottom right
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(600, 450);
        glVertex2f(750, 450);
        glVertex2f(750, 480);
        glVertex2f(600, 480);
        glEnd();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    void renderHUD() {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Crosshair
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex2f(395, 300);
        glVertex2f(405, 300);
        glVertex2f(400, 295);
        glVertex2f(400, 305);
        glEnd();

        // Health bar
        glColor3f(1.0f, 0.2f, 0.2f);
        float healthPercent = player.health / 440.0f;
        glBegin(GL_QUADS);
        glVertex2f(20, 550);
        glVertex2f(20 + healthPercent * 200, 550);
        glVertex2f(20 + healthPercent * 200, 570);
        glVertex2f(20, 570);
        glEnd();

        // Extraction indicator
        if (currentExtraction) {
            glColor3f(0.2f, 1.0f, 0.2f);
            float progress = currentExtraction->extractProgress / currentExtraction->extractTime;
            glBegin(GL_QUADS);
            glVertex2f(300, 500);
            glVertex2f(300 + progress * 200, 500);
            glVertex2f(300 + progress * 200, 520);
            glVertex2f(300, 520);
            glEnd();
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
    }

    void renderInventoryUI() {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Semi-transparent background
        glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(800, 0);
        glVertex2f(800, 600);
        glVertex2f(0, 600);
        glEnd();

        // Inventory grid (simplified)
        glColor3f(0.3f, 0.3f, 0.3f);
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 6; x++) {
                glBegin(GL_LINE_LOOP);
                glVertex2f(250 + x * 50, 100 + y * 50);
                glVertex2f(300 + x * 50, 100 + y * 50);
                glVertex2f(300 + x * 50, 150 + y * 50);
                glVertex2f(250 + x * 50, 150 + y * 50);
                glEnd();
            }
        }

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
    }

    void spawnLoot(int count) {
        auto& db = ItemDatabase::getInstance();
        std::vector<std::string> lootTable = {
            "ak74", "glock17", "ifak", "ai2", "water", "tushonka",
            "545x39", "9x18", "rolex", "bolts", "wires"
        };

        srand(time(0));

        for (int i = 0; i < count; i++) {
            float x = 50 + (rand() % 400);
            float z = 50 + (rand() % 400);
            float y = terrain.getHeightAt(x, z);

            std::string itemId = lootTable[rand() % lootTable.size()];
            auto item = db.createItem(itemId);

            lootSpawns.push_back(LootSpawn(x, y, z, item));
        }
    }

    void spawnEnemies(int count) {
        srand(time(0) + 1000);

        for (int i = 0; i < count; i++) {
            float x = 50 + (rand() % 400);
            float z = 50 + (rand() % 400);
            float y = terrain.getHeightAt(x, z);

            enemies.push_back(AIEnemy(x, y, z));
        }
    }

    void tryPickupLoot() {
        float pickupRadius = 2.0f;

        for (auto& loot : lootSpawns) {
            if (!loot.collected) {
                float dx = player.position.x - loot.x;
                float dz = player.position.z - loot.z;
                float dist = sqrt(dx * dx + dz * dz);

                if (dist < pickupRadius) {
                    // Pick up item
                    collectedLoot.push_back(loot.item);
                    loot.collected = true;
                    std::cout << "Picked up: " << loot.item->name << "\n";
                    return;
                }
            }
        }
    }

    void tryExtract() {
        if (currentExtraction) {
            currentState = GameState::EXTRACTING;
            std::cout << "Extracting at " << currentExtraction->name << "...\n";
        }
    }
};
