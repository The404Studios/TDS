#include "GameEngine.h"
#include "scene/SceneManager.h"
#include <iostream>

GameEngine& GameEngine::getInstance() {
    static GameEngine instance;
    return instance;
}

GameEngine::GameEngine()
    : running(false), deltaTime(0.0f), totalTime(0.0f), fps(0), frameCount(0), fpsTimer(0.0f) {
}

GameEngine::~GameEngine() {
    shutdown();
}

bool GameEngine::initialize(const EngineConfig& cfg) {
    std::cout << "[GameEngine] Initializing..." << std::endl;
    config = cfg;

    // Initialize scene manager (required for game scenes)
    sceneManager = std::make_unique<SceneManager>();
    std::cout << "[GameEngine] SceneManager initialized" << std::endl;

    // NOTE: Other subsystems (RenderEngine, PhysicsEngine, etc.) are not initialized
    // This is a minimal stub for scene management only
    // Full initialization would require platform-specific setup

    running = true;
    std::cout << "[GameEngine] Initialization complete (minimal mode)" << std::endl;
    return true;
}

void GameEngine::shutdown() {
    std::cout << "[GameEngine] Shutting down..." << std::endl;

    if (sceneManager) {
        sceneManager->unloadAll();
        sceneManager.reset();
    }

    // Clear other subsystems
    weatherSystem.reset();
    particleSystem.reset();
    audioEngine.reset();
    renderEngine.reset();
    physicsEngine.reset();
    networkEngine.reset();
    resourceManager.reset();

    running = false;
    std::cout << "[GameEngine] Shutdown complete" << std::endl;
}

void GameEngine::run() {
    // NOTE: This is a stub - actual game loop would be in main.cpp
    std::cout << "[GameEngine] run() called - not implemented (use main loop in client)" << std::endl;
}

void GameEngine::update(float dt) {
    deltaTime = dt;
    totalTime += dt;

    // Update FPS counter
    frameCount++;
    fpsTimer += dt;
    if (fpsTimer >= 1.0f) {
        fps = frameCount;
        frameCount = 0;
        fpsTimer = 0.0f;
    }

    // Update scene manager if it exists
    if (sceneManager && sceneManager->hasActive()) {
        sceneManager->tick(dt, config.fixedTimeStep);
    }
}

void GameEngine::render() {
    // Scene manager handles rendering in tick()
    // This is called from client main loop
}

void GameEngine::setVSync(bool enabled) {
    config.vsync = enabled;
    // TODO: Apply to rendering context
}

void GameEngine::setFullscreen(bool enabled) {
    config.fullscreen = enabled;
    // TODO: Apply to window
}
