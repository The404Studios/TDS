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
#include <algorithm>

// Bullet tracer for visual feedback
struct BulletTracer {
    float startX, startY, startZ;
    float endX, endY, endZ;
    float lifetime;
    float maxLifetime;

    BulletTracer(float sx, float sy, float sz, float ex, float ey, float ez)
        : startX(sx), startY(sy), startZ(sz), endX(ex), endY(ey), endZ(ez),
          lifetime(0.0f), maxLifetime(0.15f) {}

    bool isExpired() const { return lifetime >= maxLifetime; }
    void update(float dt) { lifetime += dt; }
};

// Muzzle flash effect
struct MuzzleFlash {
    float x, y, z;
    float lifetime;
    float maxLifetime;

    MuzzleFlash(float x, float y, float z)
        : x(x), y(y), z(z), lifetime(0.0f), maxLifetime(0.05f) {}

    bool isExpired() const { return lifetime >= maxLifetime; }
    void update(float dt) { lifetime += dt; }
};

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

// AI enemy with combat behavior
struct AIEnemy {
    float x, y, z;
    float health;
    float maxHealth;
    float yaw;               // Direction facing
    bool alive;
    bool aggroed;            // Has seen player
    float shootTimer;        // Time until next shot
    float shootCooldown;     // Time between shots
    float detectionRange;    // How far they can see
    float accuracy;          // Shot accuracy (0-1)
    std::vector<std::shared_ptr<Item>> loot;  // Items dropped on death

    AIEnemy(float x, float y, float z)
        : x(x), y(y), z(z), health(100.0f), maxHealth(100.0f), yaw(0.0f),
          alive(true), aggroed(false), shootTimer(0.0f), shootCooldown(2.0f),
          detectionRange(50.0f), accuracy(0.3f) {

        // Generate random loot
        auto& db = ItemDatabase::getInstance();
        int lootCount = 2 + (rand() % 4); // 2-5 items

        std::vector<std::string> possibleLoot = {
            "ak74", "kedr", "glock17", "paca", "ssh68", "scav",
            "ai2", "water", "tushonka", "bolts", "wires", "545x39", "9x18"
        };

        for (int i = 0; i < lootCount; i++) {
            std::string itemId = possibleLoot[rand() % possibleLoot.size()];
            auto item = db.createItem(itemId);
            if (item) {
                loot.push_back(item);
            }
        }
    }

    void takeDamage(float damage) {
        health -= damage;
        aggroed = true; // Aggro on damage
        if (health <= 0) {
            health = 0;
            alive = false;
        }
    }

    void update(float deltaTime, float playerX, float playerY, float playerZ) {
        if (!alive) return;

        // Calculate distance to player
        float dx = playerX - x;
        float dy = playerY - y;
        float dz = playerZ - z;
        float dist = sqrt(dx * dx + dy * dy + dz * dz);

        // Detect player
        if (dist < detectionRange) {
            aggroed = true;
        }

        if (aggroed) {
            // Face player
            yaw = atan2(dx, -dz) * 180.0f / 3.14159f;

            // Shoot at player
            shootTimer -= deltaTime;
        }
    }

    bool canShoot() const {
        return alive && aggroed && shootTimer <= 0.0f;
    }

    void resetShootTimer() {
        shootTimer = shootCooldown + (rand() % 100) / 100.0f; // Add randomness
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

    // Combat state
    std::vector<BulletTracer> tracers;
    std::vector<MuzzleFlash> muzzleFlashes;
    float fireRateTimer;     // Cooldown between shots
    int currentAmmo;         // Current magazine ammo
    int reserveAmmo;         // Reserve ammo
    bool isReloading;
    float reloadTimer;
    int playerKills;         // Kills this raid

    // Camera controls
    float cameraYaw;
    float cameraPitch;
    float cameraSensitivity;

    // Input state
    bool keys[256];
    bool mouseButtons[3];    // Left, Right, Middle
    int mouseX, mouseY;
    int lastMouseX, lastMouseY;

    // UI state
    bool inventoryOpen;
    std::shared_ptr<Item> selectedItem;  // Item being dragged
    int dragStartX, dragStartY;
    std::string statusMessage;
    float statusMessageTimer;

    ExtractionShooter()
        : currentState(GameState::MAIN_MENU),
          terrain(512, 512, 2.0f),
          raidTime(0.0f),
          maxRaidTime(30.0f * 60.0f),
          currentExtraction(nullptr),
          fireRateTimer(0.0f),
          currentAmmo(30),
          reserveAmmo(90),
          isReloading(false),
          reloadTimer(0.0f),
          playerKills(0),
          cameraYaw(0.0f),
          cameraPitch(0.0f),
          cameraSensitivity(0.2f),
          mouseX(0), mouseY(0),
          lastMouseX(0), lastMouseY(0),
          inventoryOpen(false),
          selectedItem(nullptr),
          dragStartX(0), dragStartY(0),
          statusMessageTimer(0.0f) {

        memset(keys, 0, sizeof(keys));
        memset(mouseButtons, 0, sizeof(mouseButtons));

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
        tracers.clear();
        muzzleFlashes.clear();
        playerKills = 0;

        // Respawn player at starting position
        player.position = Vector3(256.0f, 50.0f, 256.0f);
        player.health = 440.0f;
        player.isDead = false;

        // Set ammo from loadout
        if (playerData.loadout.primaryWeapon) {
            currentAmmo = playerData.loadout.primaryWeapon->magazineSize;
            reserveAmmo = currentAmmo * 3;
        }

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
        std::cout << "Extractions: ";
        for (auto& ext : extractions) {
            std::cout << ext.name << ", ";
        }
        std::cout << "\n";
        showStatusMessage("Raid started! Find loot and extract!");
    }

    void showStatusMessage(const std::string& msg) {
        statusMessage = msg;
        statusMessageTimer = 3.0f;
    }

    void update(float deltaTime) {
        // Update status message timer
        if (statusMessageTimer > 0.0f) {
            statusMessageTimer -= deltaTime;
        }

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
                    if (key == 'r' || key == 'R') startReload();
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

    void handleMouseButton(int button, bool pressed) {
        if (button >= 0 && button < 3) {
            mouseButtons[button] = pressed;

            // Left click = shoot
            if (button == 0 && pressed && currentState == GameState::IN_RAID && !inventoryOpen) {
                tryShoot();
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

        // Update fire rate timer
        if (fireRateTimer > 0.0f) {
            fireRateTimer -= deltaTime;
        }

        // Update reload
        if (isReloading) {
            reloadTimer -= deltaTime;
            if (reloadTimer <= 0.0f) {
                finishReload();
            }
        }

        // Auto-fire if mouse held down
        if (mouseButtons[0] && fireRateTimer <= 0.0f && !isReloading) {
            tryShoot();
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

        // Clamp to terrain bounds
        player.position.x = std::max(5.0f, std::min(player.position.x, 507.0f));
        player.position.z = std::max(5.0f, std::min(player.position.z, 507.0f));

        // Snap to terrain
        player.position.y = terrain.getHeightAt(player.position.x, player.position.z) + 1.8f;

        // Update player
        player.update(deltaTime, terrain);

        // Update AI enemies
        for (auto& enemy : enemies) {
            enemy.update(deltaTime, player.position.x, player.position.y, player.position.z);

            // Enemy shoots at player
            if (enemy.canShoot()) {
                enemyShoot(enemy);
                enemy.resetShootTimer();
            }
        }

        // Update tracers
        tracers.erase(
            std::remove_if(tracers.begin(), tracers.end(),
                [deltaTime](BulletTracer& t) {
                    t.update(deltaTime);
                    return t.isExpired();
                }),
            tracers.end()
        );

        // Update muzzle flashes
        muzzleFlashes.erase(
            std::remove_if(muzzleFlashes.begin(), muzzleFlashes.end(),
                [deltaTime](MuzzleFlash& f) {
                    f.update(deltaTime);
                    return f.isExpired();
                }),
            muzzleFlashes.end()
        );

        // Check for extraction zones
        for (auto& ext : extractions) {
            if (ext.active && ext.isPlayerInZone(player.position.x, player.position.z)) {
                currentExtraction = &ext;
                return;
            }
        }
        currentExtraction = nullptr;

        // Check if player died
        if (player.isDead || player.health <= 0) {
            currentState = GameState::DEATH_SCREEN;
            playerData.handleDeath();
            playerData.save("playerdata.txt");
            showStatusMessage("You died! All gear lost.");
        }
    }

    void tryShoot() {
        if (isReloading || fireRateTimer > 0.0f) return;

        if (currentAmmo <= 0) {
            showStatusMessage("Out of ammo! Press R to reload");
            return;
        }

        // Consume ammo
        currentAmmo--;
        fireRateTimer = 0.1f; // 600 RPM

        // Get weapon damage
        float damage = 40.0f;
        if (playerData.loadout.primaryWeapon) {
            damage = playerData.loadout.primaryWeapon->damage;
        }

        // Calculate shot direction
        float radPitch = cameraPitch * 3.14159f / 180.0f;
        float radYaw = cameraYaw * 3.14159f / 180.0f;

        float dirX = cos(radPitch) * sin(radYaw);
        float dirY = sin(radPitch);
        float dirZ = -cos(radPitch) * cos(radYaw);

        // Raycast for hit
        float range = 200.0f;
        float startX = player.position.x;
        float startY = player.position.y + 1.6f; // Eye height
        float startZ = player.position.z;
        float endX = startX + dirX * range;
        float endY = startY + dirY * range;
        float endZ = startZ + dirZ * range;

        // Check enemy hits
        bool hitEnemy = false;
        for (auto& enemy : enemies) {
            if (!enemy.alive) continue;

            // Simple sphere collision
            float dx = enemy.x - startX;
            float dy = (enemy.y + 0.9f) - startY; // Center of enemy
            float dz = enemy.z - startZ;

            // Project onto ray
            float dot = dx * dirX + dy * dirY + dz * dirZ;
            if (dot < 0 || dot > range) continue;

            float closestX = startX + dirX * dot;
            float closestY = startY + dirY * dot;
            float closestZ = startZ + dirZ * dot;

            float distSq = (enemy.x - closestX) * (enemy.x - closestX) +
                          ((enemy.y + 0.9f) - closestY) * ((enemy.y + 0.9f) - closestY) +
                          (enemy.z - closestZ) * (enemy.z - closestZ);

            if (distSq < 1.0f) { // Hit radius
                enemy.takeDamage(damage);
                hitEnemy = true;

                if (!enemy.alive) {
                    playerKills++;
                    showStatusMessage("Enemy killed!");

                    // Drop loot at enemy position
                    for (auto& item : enemy.loot) {
                        lootSpawns.push_back(LootSpawn(enemy.x, enemy.y, enemy.z, item));
                    }
                }

                // Adjust end point to enemy
                endX = enemy.x;
                endY = enemy.y + 0.9f;
                endZ = enemy.z;
                break;
            }
        }

        // Add bullet tracer
        tracers.push_back(BulletTracer(startX, startY, startZ, endX, endY, endZ));

        // Add muzzle flash
        muzzleFlashes.push_back(MuzzleFlash(startX, startY, startZ));
    }

    void enemyShoot(AIEnemy& enemy) {
        // Calculate direction to player
        float dx = player.position.x - enemy.x;
        float dy = (player.position.y + 0.9f) - (enemy.y + 0.9f);
        float dz = player.position.z - enemy.z;

        float len = sqrt(dx * dx + dy * dy + dz * dz);
        if (len < 0.01f) return;

        dx /= len;
        dy /= len;
        dz /= len;

        // Apply accuracy (randomize direction)
        float spread = 1.0f - enemy.accuracy;
        dx += ((rand() % 200 - 100) / 100.0f) * spread;
        dy += ((rand() % 200 - 100) / 100.0f) * spread;
        dz += ((rand() % 200 - 100) / 100.0f) * spread;

        // Normalize again
        len = sqrt(dx * dx + dy * dy + dz * dz);
        dx /= len;
        dy /= len;
        dz /= len;

        float range = 100.0f;
        float startX = enemy.x;
        float startY = enemy.y + 1.4f;
        float startZ = enemy.z;
        float endX = startX + dx * range;
        float endY = startY + dy * range;
        float endZ = startZ + dz * range;

        // Check if hit player
        float distToPlayer = sqrt(
            (player.position.x - startX) * (player.position.x - startX) +
            ((player.position.y + 0.9f) - startY) * ((player.position.y + 0.9f) - startY) +
            (player.position.z - startZ) * (player.position.z - startZ)
        );

        float dot = (player.position.x - startX) * dx +
                   ((player.position.y + 0.9f) - startY) * dy +
                   (player.position.z - startZ) * dz;

        if (dot > 0 && dot < range) {
            float closestX = startX + dx * dot;
            float closestY = startY + dy * dot;
            float closestZ = startZ + dz * dot;

            float distSq = (player.position.x - closestX) * (player.position.x - closestX) +
                          ((player.position.y + 0.9f) - closestY) * ((player.position.y + 0.9f) - closestY) +
                          (player.position.z - closestZ) * (player.position.z - closestZ);

            if (distSq < 1.5f) { // Hit player
                float damage = 25.0f;
                player.health -= damage;
                showStatusMessage("You were hit!");

                if (player.health <= 0) {
                    player.isDead = true;
                }

                endX = player.position.x;
                endY = player.position.y + 0.9f;
                endZ = player.position.z;
            }
        }

        // Add enemy tracer (red)
        tracers.push_back(BulletTracer(startX, startY, startZ, endX, endY, endZ));
    }

    void startReload() {
        if (isReloading || currentAmmo == 30) return; // Already full
        if (reserveAmmo <= 0) {
            showStatusMessage("No reserve ammo!");
            return;
        }

        isReloading = true;
        reloadTimer = 2.5f; // 2.5 second reload
        showStatusMessage("Reloading...");
    }

    void finishReload() {
        int needed = 30 - currentAmmo;
        int refill = std::min(needed, reserveAmmo);
        currentAmmo += refill;
        reserveAmmo -= refill;
        isReloading = false;
        showStatusMessage("Reload complete");
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

            playerData.playerData.kills += playerKills;
            playerData.save("playerdata.txt");
            showStatusMessage("Extracted successfully!");
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
        glDisable(GL_LIGHTING);
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

        // Title
        glColor3f(0.8f, 0.6f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(200, 100);
        glVertex2f(600, 100);
        glVertex2f(600, 180);
        glVertex2f(200, 180);
        glEnd();

        // Menu options
        glColor3f(0.3f, 0.3f, 0.4f);
        // Option 1: Stash
        glBegin(GL_QUADS);
        glVertex2f(300, 250);
        glVertex2f(500, 250);
        glVertex2f(500, 300);
        glVertex2f(300, 300);
        glEnd();

        glColor3f(0.35f, 0.35f, 0.45f);
        // Option 2: Raid
        glBegin(GL_QUADS);
        glVertex2f(300, 320);
        glVertex2f(500, 320);
        glVertex2f(500, 370);
        glVertex2f(300, 370);
        glEnd();

        // Player stats
        glColor3f(0.6f, 0.6f, 0.6f);
        glBegin(GL_QUADS);
        glVertex2f(50, 400);
        glVertex2f(350, 400);
        glVertex2f(350, 550);
        glVertex2f(50, 550);
        glEnd();

        // Restore
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
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

        // Render bullet tracers
        renderTracers();

        // Render muzzle flashes
        renderMuzzleFlashes();

        // Render HUD
        renderHUD();

        if (inventoryOpen) {
            renderInventoryUI();
        }
    }

    void renderExtracting() {
        renderInRaid();  // Same as in raid, but extraction UI is part of HUD
    }

    void renderPostRaid() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Success background
        glColor3f(0.1f, 0.2f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(800, 0);
        glVertex2f(800, 600);
        glVertex2f(0, 600);
        glEnd();

        // Success banner
        glColor3f(0.2f, 0.8f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(200, 100);
        glVertex2f(600, 100);
        glVertex2f(600, 180);
        glVertex2f(200, 180);
        glEnd();

        // Loot summary box
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glVertex2f(150, 220);
        glVertex2f(650, 220);
        glVertex2f(650, 500);
        glVertex2f(150, 500);
        glEnd();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    void renderDeathScreen() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        // Death background
        glColor3f(0.2f, 0.05f, 0.05f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(800, 0);
        glVertex2f(800, 600);
        glVertex2f(0, 600);
        glEnd();

        // Death banner
        glColor3f(0.8f, 0.1f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2f(200, 200);
        glVertex2f(600, 200);
        glVertex2f(600, 280);
        glVertex2f(200, 280);
        glEnd();

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    void renderLootSpawns() {
        glDisable(GL_LIGHTING);
        for (auto& loot : lootSpawns) {
            if (!loot.collected) {
                glPushMatrix();
                glTranslatef(loot.x, loot.y + 0.5f, loot.z);

                // Draw as rotating cube based on rarity
                Color rarityColor = getRarityColor(loot.item->rarity);
                glColor3f(rarityColor.r, rarityColor.g, rarityColor.b);

                float size = 0.25f;
                glBegin(GL_QUADS);
                // Front
                glVertex3f(-size, -size, size);
                glVertex3f(size, -size, size);
                glVertex3f(size, size, size);
                glVertex3f(-size, size, size);
                // Back
                glVertex3f(-size, -size, -size);
                glVertex3f(size, -size, -size);
                glVertex3f(size, size, -size);
                glVertex3f(-size, size, -size);
                // Top
                glVertex3f(-size, size, -size);
                glVertex3f(size, size, -size);
                glVertex3f(size, size, size);
                glVertex3f(-size, size, size);
                // Bottom
                glVertex3f(-size, -size, -size);
                glVertex3f(size, -size, -size);
                glVertex3f(size, -size, size);
                glVertex3f(-size, -size, size);
                // Right
                glVertex3f(size, -size, -size);
                glVertex3f(size, size, -size);
                glVertex3f(size, size, size);
                glVertex3f(size, -size, size);
                // Left
                glVertex3f(-size, -size, -size);
                glVertex3f(-size, size, -size);
                glVertex3f(-size, size, size);
                glVertex3f(-size, -size, size);
                glEnd();

                glPopMatrix();
            }
        }
        glEnable(GL_LIGHTING);
    }

    Color getRarityColor(ItemRarity rarity) {
        switch (rarity) {
            case ItemRarity::COMMON: return Color(0.7f, 0.7f, 0.7f);
            case ItemRarity::UNCOMMON: return Color(0.2f, 0.8f, 0.2f);
            case ItemRarity::RARE: return Color(0.2f, 0.5f, 1.0f);
            case ItemRarity::EPIC: return Color(0.7f, 0.2f, 0.9f);
            case ItemRarity::LEGENDARY: return Color(1.0f, 0.6f, 0.0f);
            default: return Color(1.0f, 1.0f, 1.0f);
        }
    }

    void renderExtractionZones() {
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
    }

    void renderEnemies() {
        glDisable(GL_LIGHTING);
        for (auto& enemy : enemies) {
            if (enemy.alive) {
                glPushMatrix();
                glTranslatef(enemy.x, enemy.y, enemy.z);

                // Draw as red capsule/cylinder
                Color enemyColor = enemy.aggroed ? Color(1.0f, 0.0f, 0.0f) : Color(0.8f, 0.4f, 0.0f);
                glColor3f(enemyColor.r, enemyColor.g, enemyColor.b);

                // Simple rectangular body
                glBegin(GL_QUADS);
                // Front
                glVertex3f(-0.3f, 0.0f, 0.3f);
                glVertex3f(0.3f, 0.0f, 0.3f);
                glVertex3f(0.3f, 1.8f, 0.3f);
                glVertex3f(-0.3f, 1.8f, 0.3f);
                // Back
                glVertex3f(-0.3f, 0.0f, -0.3f);
                glVertex3f(0.3f, 0.0f, -0.3f);
                glVertex3f(0.3f, 1.8f, -0.3f);
                glVertex3f(-0.3f, 1.8f, -0.3f);
                glEnd();

                glPopMatrix();
            }
        }
        glEnable(GL_LIGHTING);
    }

    void renderTracers() {
        glDisable(GL_LIGHTING);
        glLineWidth(2.0f);

        for (auto& tracer : tracers) {
            float alpha = 1.0f - (tracer.lifetime / tracer.maxLifetime);
            glColor4f(1.0f, 0.9f, 0.5f, alpha);

            glBegin(GL_LINES);
            glVertex3f(tracer.startX, tracer.startY, tracer.startZ);
            glVertex3f(tracer.endX, tracer.endY, tracer.endZ);
            glEnd();
        }

        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }

    void renderMuzzleFlashes() {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        for (auto& flash : muzzleFlashes) {
            float alpha = 1.0f - (flash.lifetime / flash.maxLifetime);
            glColor4f(1.0f, 0.8f, 0.2f, alpha);

            glPushMatrix();
            glTranslatef(flash.x, flash.y, flash.z);

            // Draw as billboard sprite
            float size = 0.5f;
            glBegin(GL_QUADS);
            glVertex3f(-size, -size, 0);
            glVertex3f(size, -size, 0);
            glVertex3f(size, size, 0);
            glVertex3f(-size, size, 0);
            glEnd();

            glPopMatrix();
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    void renderHUD() {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Crosshair
        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(395, 300);
        glVertex2f(405, 300);
        glVertex2f(400, 295);
        glVertex2f(400, 305);
        glEnd();

        // Health bar background
        glColor3f(0.2f, 0.2f, 0.2f);
        glBegin(GL_QUADS);
        glVertex2f(18, 548);
        glVertex2f(222, 548);
        glVertex2f(222, 572);
        glVertex2f(18, 572);
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

        // Ammo counter
        glColor3f(0.9f, 0.9f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(700, 550);
        glVertex2f(780, 550);
        glVertex2f(780, 570);
        glVertex2f(700, 570);
        glEnd();

        // Reload indicator
        if (isReloading) {
            glColor3f(1.0f, 0.5f, 0.0f);
            float reloadPercent = 1.0f - (reloadTimer / 2.5f);
            glBegin(GL_QUADS);
            glVertex2f(700, 530);
            glVertex2f(700 + reloadPercent * 80, 530);
            glVertex2f(700 + reloadPercent * 80, 545);
            glVertex2f(700, 545);
            glEnd();
        }

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

        // Status message
        if (statusMessageTimer > 0.0f) {
            glColor4f(1.0f, 1.0f, 1.0f, std::min(1.0f, statusMessageTimer));
            glBegin(GL_QUADS);
            glVertex2f(250, 450);
            glVertex2f(550, 450);
            glVertex2f(550, 480);
            glVertex2f(250, 480);
            glEnd();
        }

        // Kill counter
        glColor3f(0.8f, 0.8f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(20, 20);
        glVertex2f(100, 20);
        glVertex2f(100, 40);
        glVertex2f(20, 40);
        glEnd();

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    void renderInventoryUI() {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 800, 600, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        // Semi-transparent background
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(800, 0);
        glVertex2f(800, 600);
        glVertex2f(0, 600);
        glEnd();

        // Inventory grid (simplified)
        glColor3f(0.3f, 0.3f, 0.3f);
        int gridSize = 50;
        for (int y = 0; y < 8; y++) {
            for (int x = 0; x < 6; x++) {
                glBegin(GL_LINE_LOOP);
                glVertex2f(250 + x * gridSize, 100 + y * gridSize);
                glVertex2f(250 + (x+1) * gridSize, 100 + y * gridSize);
                glVertex2f(250 + (x+1) * gridSize, 100 + (y+1) * gridSize);
                glVertex2f(250 + x * gridSize, 100 + (y+1) * gridSize);
                glEnd();
            }
        }

        glDisable(GL_BLEND);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    void spawnLoot(int count) {
        auto& db = ItemDatabase::getInstance();
        std::vector<std::string> lootTable = {
            "ak74", "m4a1", "glock17", "kedr", "ifak", "ai2", "grizzly",
            "water", "tushonka", "545x39", "556x45", "9x18",
            "paca", "6b3", "ssh68", "zsh", "scav", "berkut",
            "rolex", "bitcoin", "ledx", "gpu", "bolts", "wires", "gunpowder"
        };

        srand(time(0));

        for (int i = 0; i < count; i++) {
            float x = 50 + (rand() % 412);
            float z = 50 + (rand() % 412);
            float y = terrain.getHeightAt(x, z);

            std::string itemId = lootTable[rand() % lootTable.size()];
            auto item = db.createItem(itemId);

            if (item) {
                lootSpawns.push_back(LootSpawn(x, y, z, item));
            }
        }
    }

    void spawnEnemies(int count) {
        srand(time(0) + 1000);

        for (int i = 0; i < count; i++) {
            float x = 50 + (rand() % 412);
            float z = 50 + (rand() % 412);
            float y = terrain.getHeightAt(x, z);

            enemies.push_back(AIEnemy(x, y, z));
        }
    }

    void tryPickupLoot() {
        float pickupRadius = 3.0f;

        for (auto& loot : lootSpawns) {
            if (!loot.collected) {
                float dx = player.position.x - loot.x;
                float dz = player.position.z - loot.z;
                float dist = sqrt(dx * dx + dz * dz);

                if (dist < pickupRadius) {
                    // Pick up item
                    collectedLoot.push_back(loot.item);
                    loot.collected = true;
                    showStatusMessage("Picked up: " + loot.item->name);
                    return;
                }
            }
        }
    }

    void tryExtract() {
        if (currentExtraction) {
            currentState = GameState::EXTRACTING;
            showStatusMessage("Extracting at " + currentExtraction->name + "...");
        }
    }
};
