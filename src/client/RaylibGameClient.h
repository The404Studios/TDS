// Raylib-based 3D Game Client with raygui UI
// Main game client using raylib for 3D rendering and raygui for UI

#pragma once

// Use platform-aware raylib header to avoid Windows conflicts
#include "RaylibPlatform.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "network/NetworkClient.h"
#include "animation/AnimationController.h"
#include "../common/CorpseSystem.h"
#include "../common/DataStructures.h"
#include <vector>
#include <unordered_map>
#include <string>

// Game state enum
enum class GameState {
    LOGIN,
    MAIN_MENU,
    IN_RAID,
    INVENTORY,
    MERCHANT,
    DEAD,
    EXTRACTED
};

// Represent other players in the world
struct RemotePlayer {
    uint64_t playerId;
    std::string playerName;
    Vector3 position;
    float rotation;
    AnimationController animController;
    bool isAlive;

    RemotePlayer() : playerId(0), rotation(0.0f), isAlive(true) {
        position = { 0, 0, 0 };
    }
};

class RaylibGameClient {
private:
    // Window and camera
    Camera3D m_camera;
    int m_screenWidth;
    int m_screenHeight;

    // Networking
    NetworkClient* m_networkClient;
    uint64_t m_accountId;
    std::string m_username;

    // Game state
    GameState m_state;
    bool m_inRaid;
    bool m_isDead;
    bool m_isExtracted;

    // Player
    Vector3 m_playerPosition;
    float m_playerRotation;
    float m_playerHealth;
    float m_playerMaxHealth;
    AnimationController m_playerAnimController;
    PlayerAnimationStateMachine m_playerAnimStateMachine;

    // Movement state
    bool m_isMoving;
    bool m_isRunning;
    bool m_isCrouching;
    bool m_isShooting;
    bool m_isReloading;
    bool m_isLooting;

    // Remote players
    std::unordered_map<uint64_t, RemotePlayer> m_remotePlayers;

    // Corpses
    std::vector<ClientCorpse> m_corpses;
    uint64_t m_selectedCorpseId;

    // UI state
    char m_usernameInput[64];
    char m_passwordInput[64];
    bool m_showInventory;
    int m_selectedInventorySlot;

    // Timing
    float m_networkUpdateTimer;
    static constexpr float NETWORK_UPDATE_INTERVAL = 0.1f; // 10 Hz

    // World
    Model m_groundModel;
    Texture2D m_groundTexture;

public:
    RaylibGameClient(NetworkClient* netClient, int screenWidth = 1280, int screenHeight = 720)
        : m_networkClient(netClient)
        , m_screenWidth(screenWidth)
        , m_screenHeight(screenHeight)
        , m_accountId(0)
        , m_state(GameState::LOGIN)
        , m_inRaid(false)
        , m_isDead(false)
        , m_isExtracted(false)
        , m_playerPosition({ 0, 0, 0 })
        , m_playerRotation(0.0f)
        , m_playerHealth(100.0f)
        , m_playerMaxHealth(100.0f)
        , m_playerAnimStateMachine(m_playerAnimController)
        , m_isMoving(false)
        , m_isRunning(false)
        , m_isCrouching(false)
        , m_isShooting(false)
        , m_isReloading(false)
        , m_isLooting(false)
        , m_selectedCorpseId(0)
        , m_showInventory(false)
        , m_selectedInventorySlot(-1)
        , m_networkUpdateTimer(0.0f)
    {
        memset(m_usernameInput, 0, sizeof(m_usernameInput));
        memset(m_passwordInput, 0, sizeof(m_passwordInput));

        // Initialize camera
        m_camera.position = { 0.0f, 2.0f, -5.0f };
        m_camera.target = { 0.0f, 1.0f, 0.0f };
        m_camera.up = { 0.0f, 1.0f, 0.0f };
        m_camera.fovy = 60.0f;
        m_camera.projection = CAMERA_PERSPECTIVE;
    }

    ~RaylibGameClient() {
        cleanup();
    }

    void initialize() {
        TraceLog(LOG_INFO, "Initializing RaylibGameClient...");

        // Load player model and animations
        if (FileExists("resources/models/player.glb")) {
            m_playerAnimController.loadModel("resources/models/player.glb", "resources/animations/player_anims.glb");
        } else {
            TraceLog(LOG_WARNING, "Player model not found, using placeholder");
        }

        // Load ground plane
        Mesh groundMesh = GenMeshPlane(200.0f, 200.0f, 10, 10);
        m_groundModel = LoadModelFromMesh(groundMesh);

        // Load ground texture if available
        if (FileExists("resources/textures/ground.png")) {
            m_groundTexture = LoadTexture("resources/textures/ground.png");
            m_groundModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = m_groundTexture;
        }

        TraceLog(LOG_INFO, "RaylibGameClient initialized");
    }

    void update(float deltaTime) {
        // Update network
        if (m_networkClient) {
            m_networkClient->update();
        }

        switch (m_state) {
            case GameState::LOGIN:
                updateLogin();
                break;

            case GameState::MAIN_MENU:
                updateMainMenu();
                break;

            case GameState::IN_RAID:
                updateInRaid(deltaTime);
                break;

            case GameState::INVENTORY:
                updateInventory();
                break;

            case GameState::DEAD:
                updateDead();
                break;

            case GameState::EXTRACTED:
                updateExtracted();
                break;
        }
    }

    void render() {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (m_state) {
            case GameState::LOGIN:
                renderLogin();
                break;

            case GameState::MAIN_MENU:
                renderMainMenu();
                break;

            case GameState::IN_RAID:
                renderInRaid();
                break;

            case GameState::INVENTORY:
                renderInventory();
                break;

            case GameState::DEAD:
                renderDead();
                break;

            case GameState::EXTRACTED:
                renderExtracted();
                break;
        }

        // Draw FPS
        DrawFPS(10, 10);

        EndDrawing();
    }

private:
    // ========== LOGIN ==========
    void updateLogin() {
        // Handle input in renderLogin with raygui
    }

    void renderLogin() {
        DrawText("EXTRACTION SHOOTER", 400, 100, 40, DARKBLUE);
        DrawText("Multiplayer Looter Shooter with raylib", 420, 150, 20, GRAY);

        // Username input
        GuiLabel({ 400, 220, 100, 30 }, "Username:");
        if (GuiTextBox({ 520, 220, 300, 30 }, m_usernameInput, 64, true)) {
            // Text box activated
        }

        // Password input
        GuiLabel({ 400, 270, 100, 30 }, "Password:");
        if (GuiTextBox({ 520, 270, 300, 30 }, m_passwordInput, 64, true)) {
            // Text box activated
        }

        // Login button
        if (GuiButton({ 520, 330, 140, 40 }, "Login")) {
            attemptLogin();
        }

        // Register button
        if (GuiButton({ 680, 330, 140, 40 }, "Register")) {
            attemptRegister();
        }

        DrawText("Press TAB to switch between fields", 440, 400, 16, DARKGRAY);
    }

    void attemptLogin() {
        std::cout << "[Client] Attempting login: " << m_usernameInput << std::endl;
        // TODO: Send login packet via NetworkClient
        // For now, simulate success
        m_username = std::string(m_usernameInput);
        m_accountId = 12345; // Would come from server
        m_state = GameState::MAIN_MENU;
    }

    void attemptRegister() {
        std::cout << "[Client] Attempting registration: " << m_usernameInput << std::endl;
        // TODO: Send registration packet via NetworkClient
    }

    // ========== MAIN MENU ==========
    void updateMainMenu() {
        // Handle button clicks in renderMainMenu
    }

    void renderMainMenu() {
        DrawText(TextFormat("Welcome, %s", m_username.c_str()), 450, 100, 30, DARKBLUE);
        DrawText(TextFormat("Account ID: %llu", m_accountId), 480, 140, 20, GRAY);

        // Menu buttons
        if (GuiButton({ 500, 220, 280, 50 }, "Enter Raid")) {
            enterRaid();
        }

        if (GuiButton({ 500, 290, 280, 50 }, "View Stash")) {
            m_state = GameState::INVENTORY;
        }

        if (GuiButton({ 500, 360, 280, 50 }, "Merchants")) {
            m_state = GameState::MERCHANT;
        }

        if (GuiButton({ 500, 430, 280, 50 }, "Logout")) {
            m_state = GameState::LOGIN;
            memset(m_usernameInput, 0, sizeof(m_usernameInput));
            memset(m_passwordInput, 0, sizeof(m_passwordInput));
        }
    }

    void enterRaid() {
        std::cout << "[Client] Entering raid..." << std::endl;
        m_state = GameState::IN_RAID;
        m_inRaid = true;
        m_isDead = false;
        m_playerHealth = m_playerMaxHealth;
        m_playerPosition = { 0, 0, 0 };

        // Update camera
        m_camera.position = { m_playerPosition.x, m_playerPosition.y + 2.0f, m_playerPosition.z - 5.0f };
        m_camera.target = { m_playerPosition.x, m_playerPosition.y + 1.0f, m_playerPosition.z };
    }

    // ========== IN RAID ==========
    void updateInRaid(float deltaTime) {
        if (m_isDead) {
            m_state = GameState::DEAD;
            return;
        }

        // Handle player movement
        handlePlayerMovement(deltaTime);

        // Handle combat input
        handleCombatInput();

        // Handle interaction input
        handleInteractionInput();

        // Update animations
        m_playerAnimStateMachine.update(deltaTime, m_isMoving, m_isRunning, m_isCrouching,
                                        m_isShooting, m_isReloading, m_isLooting, m_isDead);

        // Update remote players
        for (auto& pair : m_remotePlayers) {
            pair.second.animController.update(deltaTime);
        }

        // Send position updates to server
        m_networkUpdateTimer += deltaTime;
        if (m_networkUpdateTimer >= NETWORK_UPDATE_INTERVAL) {
            sendPositionUpdate();
            m_networkUpdateTimer = 0.0f;
        }

        // Update camera to follow player (third-person)
        updateCamera();
    }

    void handlePlayerMovement(float deltaTime) {
        float speed = m_isRunning ? 7.0f : 3.5f;
        if (m_isCrouching) speed = 1.5f;

        Vector3 movement = { 0, 0, 0 };
        m_isMoving = false;

        if (IsKeyDown(KEY_W)) {
            movement.z += speed * deltaTime;
            m_isMoving = true;
        }
        if (IsKeyDown(KEY_S)) {
            movement.z -= speed * deltaTime;
            m_isMoving = true;
        }
        if (IsKeyDown(KEY_A)) {
            movement.x -= speed * deltaTime;
            m_isMoving = true;
        }
        if (IsKeyDown(KEY_D)) {
            movement.x += speed * deltaTime;
            m_isMoving = true;
        }

        // Apply movement
        m_playerPosition.x += movement.x;
        m_playerPosition.z += movement.z;

        // Handle running toggle
        if (IsKeyPressed(KEY_LEFT_SHIFT)) {
            m_isRunning = !m_isRunning;
        }

        // Handle crouching
        if (IsKeyPressed(KEY_C)) {
            m_isCrouching = !m_isCrouching;
        }

        // Handle rotation with mouse
        Vector2 mouseDelta = GetMouseDelta();
        m_playerRotation += mouseDelta.x * 0.1f;
    }

    void handleCombatInput() {
        // Shooting
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            m_isShooting = true;
            shootWeapon();
        } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            m_isShooting = false;
        }

        // Reloading
        if (IsKeyPressed(KEY_R)) {
            m_isReloading = true;
            reloadWeapon();
        } else if (m_isReloading && m_playerAnimController.isAnimationFinished()) {
            m_isReloading = false;
        }
    }

    void handleInteractionInput() {
        // Loot nearby corpses
        if (IsKeyPressed(KEY_F)) {
            checkCorpseLoot();
        }

        // Toggle inventory
        if (IsKeyPressed(KEY_TAB)) {
            m_showInventory = !m_showInventory;
        }

        // Extract
        if (IsKeyPressed(KEY_E)) {
            checkExtraction();
        }
    }

    void renderInRaid() {
        // 3D rendering
        BeginMode3D(m_camera);

        // Draw ground
        DrawModel(m_groundModel, { 0, 0, 0 }, 1.0f, WHITE);

        // Draw grid for reference
        DrawGrid(50, 1.0f);

        // Draw player model
        if (m_playerAnimController.getModel().meshCount > 0) {
            m_playerAnimController.draw(m_playerPosition, { 0, 1, 0 }, m_playerRotation, 1.0f);
        } else {
            // Draw placeholder cube
            DrawCube(m_playerPosition, 1.0f, 2.0f, 1.0f, BLUE);
        }

        // Draw remote players
        for (const auto& pair : m_remotePlayers) {
            const RemotePlayer& player = pair.second;
            if (player.isAlive) {
                if (player.animController.getModel().meshCount > 0) {
                    player.animController.draw(player.position, { 0, 1, 0 }, player.rotation, 1.0f);
                } else {
                    DrawCube(player.position, 1.0f, 2.0f, 1.0f, RED);
                }

                // Draw player name above head
                Vector2 screenPos = GetWorldToScreen(
                    { player.position.x, player.position.y + 2.5f, player.position.z }, m_camera);
                DrawText(player.playerName.c_str(),
                        static_cast<int>(screenPos.x) - 20, static_cast<int>(screenPos.y),
                        20, WHITE);
            }
        }

        // Draw corpses
        for (const ClientCorpse& corpse : m_corpses) {
            Vector3 corpsePos = { corpse.position[0], corpse.position[1], corpse.position[2] };

            // Draw corpse as lying down cube (placeholder)
            DrawCubeWires(corpsePos, 2.0f, 0.5f, 1.0f, DARKGRAY);

            // Draw loot indicator
            if (corpse.hasLoot) {
                DrawCube({ corpsePos.x, corpsePos.y + 0.5f, corpsePos.z }, 0.3f, 0.3f, 0.3f, GOLD);
            }
        }

        EndMode3D();

        // UI overlay
        DrawRectangle(10, m_screenHeight - 80, 300, 70, Fade(BLACK, 0.7f));
        DrawText(TextFormat("Health: %.0f / %.0f", m_playerHealth, m_playerMaxHealth), 20, m_screenHeight - 70, 20, GREEN);
        DrawText(TextFormat("Pos: (%.1f, %.1f, %.1f)", m_playerPosition.x, m_playerPosition.y, m_playerPosition.z), 20, m_screenHeight - 45, 16, WHITE);
        DrawText("WASD: Move | Shift: Sprint | C: Crouch | F: Loot | E: Extract | TAB: Inventory", 20, m_screenHeight - 20, 14, LIGHTGRAY);

        // Draw crosshair
        DrawCircle(m_screenWidth / 2, m_screenHeight / 2, 3, RED);
        DrawCircleLines(m_screenWidth / 2, m_screenHeight / 2, 15, RED);

        // Draw inventory overlay if open
        if (m_showInventory) {
            renderInventoryOverlay();
        }
    }

    void updateCamera() {
        // Third-person camera following player
        float camDistance = 5.0f;
        float camHeight = 2.5f;

        float radians = m_playerRotation * DEG2RAD;
        m_camera.position.x = m_playerPosition.x - sinf(radians) * camDistance;
        m_camera.position.y = m_playerPosition.y + camHeight;
        m_camera.position.z = m_playerPosition.z - cosf(radians) * camDistance;

        m_camera.target = { m_playerPosition.x, m_playerPosition.y + 1.0f, m_playerPosition.z };
    }

    void shootWeapon() {
        std::cout << "[Client] Shooting weapon" << std::endl;
        // TODO: Send shoot packet to server
    }

    void reloadWeapon() {
        std::cout << "[Client] Reloading weapon" << std::endl;
        // TODO: Send reload packet to server
    }

    void checkCorpseLoot() {
        // Find nearest corpse
        float minDist = 999999.0f;
        uint64_t nearestCorpse = 0;

        for (const ClientCorpse& corpse : m_corpses) {
            Vector3 corpsePos = { corpse.position[0], corpse.position[1], corpse.position[2] };
            float dist = Vector3Distance(m_playerPosition, corpsePos);

            if (dist < minDist && dist < 3.0f) {
                minDist = dist;
                nearestCorpse = corpse.corpseId;
            }
        }

        if (nearestCorpse != 0) {
            m_selectedCorpseId = nearestCorpse;
            m_isLooting = true;
            std::cout << "[Client] Looting corpse: " << nearestCorpse << std::endl;
            // TODO: Send loot request to server
        }
    }

    void checkExtraction() {
        // TODO: Check if near extraction zone and send extraction request
        std::cout << "[Client] Checking extraction..." << std::endl;

        // Simulate successful extraction for now
        m_isExtracted = true;
        m_state = GameState::EXTRACTED;
    }

    void sendPositionUpdate() {
        // TODO: Send position packet to server via NetworkClient
    }

    void renderInventoryOverlay() {
        int panelX = m_screenWidth / 2 - 300;
        int panelY = m_screenHeight / 2 - 250;

        GuiPanel({ (float)panelX, (float)panelY, 600, 500 }, "Inventory");

        // TODO: Render actual inventory items in grid
        DrawText("Inventory items will be displayed here", panelX + 50, panelY + 50, 20, DARKGRAY);

        if (GuiButton({ (float)panelX + 250, (float)panelY + 450, 100, 30 }, "Close")) {
            m_showInventory = false;
        }
    }

    // ========== INVENTORY ==========
    void updateInventory() {
        if (IsKeyPressed(KEY_ESCAPE)) {
            m_state = GameState::MAIN_MENU;
        }
    }

    void renderInventory() {
        DrawText("STASH", 550, 50, 40, DARKBLUE);
        DrawText("Your inventory and stash items", 480, 100, 20, GRAY);

        // TODO: Render grid-based inventory system

        if (GuiButton({ 550, 650, 180, 40 }, "Back to Menu")) {
            m_state = GameState::MAIN_MENU;
        }
    }

    // ========== DEAD ==========
    void updateDead() {
        if (IsKeyPressed(KEY_SPACE)) {
            m_state = GameState::MAIN_MENU;
            m_isDead = false;
        }
    }

    void renderDead() {
        ClearBackground(RED);
        DrawText("YOU DIED", 450, 300, 60, DARKRED);
        DrawText("Press SPACE to return to menu", 470, 400, 20, MAROON);
    }

    // ========== EXTRACTED ==========
    void updateExtracted() {
        if (IsKeyPressed(KEY_SPACE)) {
            m_state = GameState::MAIN_MENU;
            m_isExtracted = false;
        }
    }

    void renderExtracted() {
        ClearBackground(GREEN);
        DrawText("EXTRACTED!", 450, 300, 60, DARKGREEN);
        DrawText("You successfully extracted with your loot!", 390, 380, 24, DARKGREEN);
        DrawText("Press SPACE to return to menu", 450, 440, 20, DARKGREEN);
    }

    void cleanup() {
        if (m_playerAnimController.getModel().meshCount > 0) {
            m_playerAnimController.unload();
        }

        UnloadModel(m_groundModel);
        if (m_groundTexture.id > 0) {
            UnloadTexture(m_groundTexture);
        }
    }
};
