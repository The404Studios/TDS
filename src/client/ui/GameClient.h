#pragma once
#include "UIManager.h"
#include "../network/NetworkClient.h"
#include "../../common/NetworkProtocol.h"
#include "../../common/DataStructures.h"
#include "../../common/ItemDatabase.h"
#include <vector>
#include <cmath>
#include <map>
#include <random>

// Forward declarations to avoid redefinition with DataStructures.h
// Use the common LootSpawn from DataStructures.h

// Client-specific structs (not in common)
struct ClientLimbHealth {
    float head = 35.0f;
    float thorax = 80.0f;
    float stomach = 70.0f;
    float leftArm = 60.0f;
    float rightArm = 60.0f;
    float leftLeg = 65.0f;
    float rightLeg = 65.0f;

    float getTotalHealth() const {
        return head + thorax + stomach + leftArm + rightArm + leftLeg + rightLeg;
    }
};

// Simple loot spawn for client-side only
struct ClientLootSpawn {
    float x, y, z;
    ItemType itemType;
    bool collected;
};

// House structure (client-side only)
struct ClientHouse {
    float x, z;
    float width, height, depth;
    bool hasLoot;
    std::vector<ClientLootSpawn> loot;
};

// Enemy (client-side simplified version)
struct ClientEnemy {
    uint32_t id;
    float x, y, z;
    float yaw;
    float health;
    bool alive;
    float lastSeenPlayer;
    float patrolAngle;
};

// Extract point (client-side)
struct ClientExtractionPoint {
    float x, z;
    float radius;
    std::string name;
};

// Tree (client-side)
struct ClientTree {
    float x, z;
    float height;
    float radius;
};

// Inventory item (client-side)
struct ClientInventoryItem {
    ItemType type;
    int count;
    int maxStack;
};

// In-game client (FPS gameplay)
class GameClient : public BaseUI {
public:
    GameClient(NetworkClient* netClient, uint64_t accId);

    void update(float deltaTime) override;
    void render() override;
    void handleInput(char key) override;
    void handleMouseMove(float deltaX, float deltaY);
    void handleMouseClick(float x, float y) override;

private:
    NetworkClient* networkClient;
    uint64_t accountId;

    // Player state
    float playerX, playerY, playerZ;
    float playerYaw, playerPitch;
    float leanAngle;  // For Q/E leaning
    bool isLeaning;
    ClientLimbHealth limbHealth;
    bool alive;
    bool extracted;

    // Camera
    bool mouseCaptured;
    float mouseSensitivity;

    // Inventory
    bool showInventory;
    float inventoryAnimProgress;
    std::vector<ClientInventoryItem> inventory;
    int selectedSlot;

    // Magazine check
    bool showMagCheck;
    float magCheckTimer;
    int currentAmmo;
    int reserveAmmo;

    // Terrain (procedural generation)
    std::vector<std::vector<float>> terrainHeights;
    int terrainSize;
    float terrainScale;

    // World objects
    std::vector<ClientLootSpawn> lootSpawns;
    std::vector<ClientHouse> houses;
    std::vector<ClientExtractionPoint> extractionPoints;
    std::vector<ClientEnemy> enemies;
    std::vector<ClientTree> trees;

    // Other players (for PvP)
    std::map<uint64_t, struct OtherPlayer> otherPlayers;

    // Random generator
    std::mt19937 rng;

    // Lighting
    float timeOfDay;  // 0.0 to 24.0
    float sunAngle;

    // Initialization
    void generateTerrain();
    void generateLoot();
    void generateHouses();
    void generateExtractionPoints();
    void generateEnemies();
    void generateTrees();

    // Rendering
    void render3DWorld();
    void renderTerrain();
    void renderHouses();
    void renderTrees();
    void renderEnemies();
    void renderOtherPlayers();
    void renderLoot();
    void renderExtractionPoints();
    void renderSkybox();
    void applyLighting();

    void renderHUD();
    void renderHealthLimbSystem();
    void renderInventory();
    void renderMagCheck();
    void renderCrosshair();
    void renderCompass();

    void renderDeathScreen();
    void renderExtractedScreen();

    // Gameplay
    void updateCamera(float deltaTime);
    void updateLean(float deltaTime);
    void updateInventoryAnimation(float deltaTime);
    void updateEnemies(float deltaTime);
    void updateTimeOfDay(float deltaTime);
    void checkLootPickup();
    void checkHouseEntry();
    bool checkLineOfSight(float x1, float y1, float z1, float x2, float y2, float z2);

    // Helpers
    float getTerrainHeight(float x, float z);
    bool isInsideHouse(float x, float z);
    bool isNearExtraction();
    ClientExtractionPoint* getNearestExtraction();

    // Network
    void sendPositionUpdate();
    void handleSpawnInfo(const std::vector<uint8_t>& payload);
    void handlePlayerDamage(const std::vector<uint8_t>& payload);
    void handlePlayerDeath(const std::vector<uint8_t>& payload);
    void handleExtractionComplete(const std::vector<uint8_t>& payload);
    void handleOtherPlayerUpdate(const std::vector<uint8_t>& payload);
    void requestExtraction();
};

// Other player struct
struct OtherPlayer {
    uint64_t accountId;
    float x, y, z;
    float yaw, pitch;
    float health;
    bool alive;
};
