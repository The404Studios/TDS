#pragma once

#include <memory>
#include <string>

// Forward declarations
class RenderEngine;
class PhysicsEngine;
class ParticleSystem;
class WeatherSystem;
class NetworkEngine;
class ResourceManager;
class AudioEngine;
class SceneManager;

// Core engine configuration
struct EngineConfig {
    // Window
    int windowWidth = 1920;
    int windowHeight = 1080;
    bool fullscreen = false;
    bool vsync = true;
    int msaaSamples = 4;

    // Rendering
    bool enableShadows = true;
    int shadowResolution = 2048;
    bool enablePostProcessing = true;
    bool enableHDR = true;
    float gamma = 2.2f;

    // Physics
    float gravity = -9.81f;
    int physicsIterations = 8;
    float fixedTimeStep = 1.0f / 60.0f;

    // Networking
    bool enableNetworking = true;
    bool useNATTraversal = true;
    int serverPort = 7777;
    std::string stunServer = "stun.l.google.com:19302";

    // Audio
    int maxAudioSources = 32;
    bool enable3DAudio = true;

    // Performance
    int maxParticles = 10000;
    int maxLights = 16;
    bool enableOcclusion = true;
    bool enableLOD = true;
};

// Main game engine - manages all subsystems
class GameEngine {
public:
    static GameEngine& getInstance();

    // Initialization
    bool initialize(const EngineConfig& config);
    void shutdown();

    // Main loop
    void run();
    void update(float deltaTime);
    void render();

    // Subsystem access
    RenderEngine* getRenderEngine() { return renderEngine.get(); }
    PhysicsEngine* getPhysicsEngine() { return physicsEngine.get(); }
    ParticleSystem* getParticleSystem() { return particleSystem.get(); }
    WeatherSystem* getWeatherSystem() { return weatherSystem.get(); }
    NetworkEngine* getNetworkEngine() { return networkEngine.get(); }
    ResourceManager* getResourceManager() { return resourceManager.get(); }
    AudioEngine* getAudioEngine() { return audioEngine.get(); }
    SceneManager* getSceneManager() { return sceneManager.get(); }

    // State
    bool isRunning() const { return running; }
    void quit() { running = false; }
    float getDeltaTime() const { return deltaTime; }
    float getTotalTime() const { return totalTime; }
    int getFPS() const { return fps; }

    // Configuration
    const EngineConfig& getConfig() const { return config; }
    void setVSync(bool enabled);
    void setFullscreen(bool enabled);

private:
    GameEngine();
    ~GameEngine();

    // No copy
    GameEngine(const GameEngine&) = delete;
    GameEngine& operator=(const GameEngine&) = delete;

    // Subsystems
    std::unique_ptr<RenderEngine> renderEngine;
    std::unique_ptr<PhysicsEngine> physicsEngine;
    std::unique_ptr<ParticleSystem> particleSystem;
    std::unique_ptr<WeatherSystem> weatherSystem;
    std::unique_ptr<NetworkEngine> networkEngine;
    std::unique_ptr<ResourceManager> resourceManager;
    std::unique_ptr<AudioEngine> audioEngine;
    std::unique_ptr<SceneManager> sceneManager;

    // State
    EngineConfig config;
    bool running = false;
    float deltaTime = 0.0f;
    float totalTime = 0.0f;
    int fps = 0;
    int frameCount = 0;
    float fpsTimer = 0.0f;
};

// Helper macro for easy access
#define ENGINE GameEngine::getInstance()
