#pragma once

#include "Scene.h"
#include "ThreadPool.h"
#include "Scheduler.h"
#include <string>
#include <map>
#include <memory>
#include <functional>

// Scene Manager - Handles scene loading, unloading, and transitions
class SceneManager {
public:
    SceneManager();
    ~SceneManager();

    // Scene registration
    void registerScene(const std::string& name, std::shared_ptr<Scene> scene);
    void unregisterScene(const std::string& name);

    // Scene loading (synchronous)
    void loadScene(const std::string& name);
    void unloadScene(const std::string& name);
    void unloadAllScenes();

    // Scene loading (asynchronous)
    void loadSceneAsync(const std::string& name, std::function<void()> onComplete = nullptr);
    void unloadSceneAsync(const std::string& name, std::function<void()> onComplete = nullptr);

    // Scene transitions
    void transitionTo(const std::string& name, bool unloadCurrent = true);
    void transitionToAsync(const std::string& name, bool unloadCurrent = true, std::function<void()> onComplete = nullptr);

    // Current scene
    std::shared_ptr<Scene> getCurrentScene() const { return currentScene; }
    std::shared_ptr<Scene> getScene(const std::string& name);

    // Update/Render
    void update(float deltaTime);
    void render();

    // Thread pool and scheduler access
    ThreadPool& getThreadPool() { return threadPool; }
    Scheduler& getScheduler() { return scheduler; }

    // Check if scene is loaded
    bool isSceneLoaded(const std::string& name) const;

private:
    std::map<std::string, std::shared_ptr<Scene>> scenes;
    std::shared_ptr<Scene> currentScene;
    ThreadPool threadPool;
    Scheduler scheduler;
    bool transitioning;
};
