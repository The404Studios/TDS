#pragma once

#include "IScene.h"
#include <memory>
#include <unordered_map>
#include <string>

/**
 * SceneManager - Manages scene lifecycle and switching
 *
 * Responsibilities:
 * - Register scenes (MenuScene, RaidScene, etc.)
 * - Switch between scenes with proper cleanup/initialization
 * - Drive the update pipeline: fixedUpdate → update → render
 * - Handle deferred scene switching (safe points between frames)
 *
 * Only ONE scene is active at a time. Scene switching happens at the start
 * of the next frame to avoid mid-frame state corruption.
 */
class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    /**
     * Register a scene with the manager
     * @tparam T Scene class (must inherit from IScene)
     * @param key Unique scene identifier
     * @param args Constructor arguments for the scene
     * @return Pointer to the created scene (for initial setup)
     */
    template<class T, class... Args>
    T* registerScene(const std::string& key, Args&&... args) {
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T* raw = ptr.get();
        scenes[key] = std::move(ptr);
        return raw;
    }

    /**
     * Request a scene switch
     * Actual switch happens at the start of the next tick() call
     * @param key Scene identifier
     * @return true if scene exists, false otherwise
     */
    bool switchTo(const std::string& key);

    /**
     * Check if a scene is currently active
     */
    bool hasActive() const { return active != nullptr; }

    /**
     * Get the currently active scene
     */
    IScene* current() const { return active; }

    /**
     * Main update loop - call this from GameEngine::update()
     * Handles fixed timestep accumulation and scene switching
     * @param dt Frame delta time
     * @param fixedDt Fixed timestep (e.g., 1/60)
     */
    void tick(float dt, float fixedDt);

    /**
     * Unload all scenes
     */
    void unloadAll();

private:
    /**
     * Perform the actual scene switch
     * Calls onExit() on old scene, onEnter() on new scene
     */
    void doSwitch();

    std::unordered_map<std::string, std::unique_ptr<IScene>> scenes;
    IScene* active;
    std::string pendingSwitch;
    float accumulator;
};
