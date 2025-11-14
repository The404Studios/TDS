#include "Game.h"
#include "Network/NetworkClient.h"
#include "Rendering/Renderer.h"
#include "Rendering/Camera.h"
#include "Rendering/ModelManager.h"
#include "UI/UIManager.h"
#include "Gameplay/Player.h"
#include "Audio/AudioManager.h"
#include "common/Items.h"

namespace TDS {

Game::Game()
    : state(GameState::LOGIN)
    , running(false)
    , deltaTime(0.0f)
    , totalTime(0.0f)
    , screenWidth(1920)
    , screenHeight(1080)
    , fullscreen(false)
    , playerId(0)
    , sessionToken(0)
{
}

Game::~Game() {
    shutdown();
}

bool Game::initialize() {
    // Initialize Raylib window
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "TDS - Tarkov Looter Shooter");
    SetTargetFPS(144);

    // Initialize Raylib audio
    InitAudioDevice();

    // Hide cursor for FPS mode (will show in menus)
    // DisableCursor();

    // Initialize item database
    ItemDatabase::initialize();

    // Create subsystems
    network = std::make_unique<NetworkClient>(this);
    renderer = std::make_unique<Renderer>(this);
    camera = std::make_unique<Camera>(this);
    ui = std::make_unique<UIManager>(this);
    player = std::make_unique<Player>(this);
    audio = std::make_unique<AudioManager>(this);
    modelManager = std::make_unique<ModelManager>(this);

    // Connect to server
    if (!network->connect("127.0.0.1", 7777)) {
        TraceLog(LOG_ERROR, "Failed to connect to server");
        return false;
    }

    TraceLog(LOG_INFO, "Game initialized successfully");
    running = true;
    return true;
}

void Game::shutdown() {
    if (network) {
        network->disconnect();
    }

    CloseAudioDevice();
    CloseWindow();

    running = false;
}

void Game::run() {
    while (running && !WindowShouldClose()) {
        deltaTime = GetFrameTime();
        totalTime += deltaTime;

        update(deltaTime);
        render();
    }
}

void Game::update(float dt) {
    // Update network
    if (network) {
        network->update();
    }

    // Update audio (music streaming)
    if (audio) {
        audio->updateMusic();
    }

    // Update based on state
    switch (state) {
        case GameState::LOGIN:
            // UI handles login
            break;

        case GameState::MAIN_MENU:
            // UI handles menu
            break;

        case GameState::LOBBY:
            // UI handles lobby
            break;

        case GameState::IN_GAME:
            // Update game systems
            if (camera) camera->update(dt);
            if (player) player->update(dt);
            break;

        case GameState::LOADING:
            // Show loading screen
            break;
    }

    // UI always updates
    if (ui) {
        ui->update(dt);
    }

    // Handle input
    handleInput();
}

void Game::render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (state) {
        case GameState::LOGIN:
        case GameState::MAIN_MENU:
        case GameState::LOBBY:
        case GameState::LOADING:
            // 2D UI rendering
            if (ui) {
                ui->render();
            }
            break;

        case GameState::IN_GAME:
            // 3D game rendering
            if (renderer && camera) {
                renderer->render();
            }
            // 3D UI overlay (HUD)
            if (ui) {
                ui->render();
            }
            break;
    }

    // Debug info
    DrawFPS(10, 10);
    DrawText(TextFormat("State: %d", (int)state), 10, 30, 20, WHITE);

    EndDrawing();
}

void Game::handleInput() {
    // Global input handling (Escape, etc.)
    if (IsKeyPressed(KEY_F11)) {
        fullscreen = !fullscreen;
        ToggleFullscreen();
    }

    // State-specific input is handled in UI and gameplay systems
}

void Game::setState(GameState newState) {
    TraceLog(LOG_INFO, "Game state changed: %d -> %d", (int)state, (int)newState);

    // Cleanup old state
    switch (state) {
        case GameState::IN_GAME:
            EnableCursor();
            break;
        default:
            break;
    }

    // Setup new state
    state = newState;
    switch (newState) {
        case GameState::LOGIN:
            EnableCursor();
            break;

        case GameState::MAIN_MENU:
            EnableCursor();
            break;

        case GameState::LOBBY:
            EnableCursor();
            break;

        case GameState::IN_GAME:
            DisableCursor();
            if (camera) {
                camera->reset();
            }
            break;

        case GameState::LOADING:
            EnableCursor();
            break;
    }

    // Notify UI of state change
    if (ui) {
        ui->onStateChanged(newState);
    }
}

} // namespace TDS
