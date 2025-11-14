#pragma once

#include "raylib.h"
#include <memory>
#include <string>

namespace TDS {

// Forward declarations
class NetworkClient;
class Renderer;
class Camera;
class UIManager;
class Player;
class AudioManager;
class ModelManager;

enum class GameState {
    LOGIN,
    MAIN_MENU,
    LOBBY,
    IN_GAME,
    LOADING,
};

class Game {
public:
    Game();
    ~Game();

    bool initialize();
    void shutdown();
    void run();

    // State management
    void setState(GameState newState);
    GameState getState() const { return state; }

    // Getters for subsystems
    NetworkClient* getNetwork() { return network.get(); }
    Renderer* getRenderer() { return renderer.get(); }
    Camera* getCamera() { return camera.get(); }
    UIManager* getUI() { return ui.get(); }
    Player* getPlayer() { return player.get(); }
    AudioManager* getAudio() { return audio.get(); }
    ModelManager* getModelManager() { return modelManager.get(); }

    // Player data
    void setPlayerId(uint32_t id) { playerId = id; }
    uint32_t getPlayerId() const { return playerId; }
    void setSessionToken(uint64_t token) { sessionToken = token; }
    uint64_t getSessionToken() const { return sessionToken; }

    // Window info
    int getScreenWidth() const { return screenWidth; }
    int getScreenHeight() const { return screenHeight; }

    // Time
    float getDeltaTime() const { return deltaTime; }
    float getTotalTime() const { return totalTime; }

private:
    void update(float dt);
    void render();
    void handleInput();

    // Subsystems
    std::unique_ptr<NetworkClient> network;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<UIManager> ui;
    std::unique_ptr<Player> player;
    std::unique_ptr<AudioManager> audio;
    std::unique_ptr<ModelManager> modelManager;

    // State
    GameState state;
    bool running;
    float deltaTime;
    float totalTime;

    // Window
    int screenWidth;
    int screenHeight;
    bool fullscreen;

    // Player session
    uint32_t playerId;
    uint64_t sessionToken;
};

} // namespace TDS
