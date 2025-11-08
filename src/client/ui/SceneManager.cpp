#include "SceneManager.h"
#include <thread>
#include <chrono>

SceneManager::SceneManager()
    : currentScene(nullptr), threadPool(4), transitioning(false) {
}

SceneManager::~SceneManager() {
    unloadAllScenes();
}

void SceneManager::registerScene(const std::string& name, std::shared_ptr<Scene> scene) {
    scenes[name] = scene;
}

void SceneManager::unregisterScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it != scenes.end()) {
        if (it->second->isLoaded()) {
            it->second->onUnload();
        }
        scenes.erase(it);
    }
}

void SceneManager::loadScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it != scenes.end() && !it->second->isLoaded()) {
        it->second->onLoad();
        currentScene = it->second;
    }
}

void SceneManager::unloadScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it != scenes.end() && it->second->isLoaded()) {
        it->second->onUnload();
        if (currentScene == it->second) {
            currentScene = nullptr;
        }
    }
}

void SceneManager::unloadAllScenes() {
    for (std::pair<const std::string, std::shared_ptr<Scene>>& scenePair : scenes) {
        if (scenePair.second->isLoaded()) {
            scenePair.second->onUnload();
        }
    }
    currentScene = nullptr;
}

void SceneManager::loadSceneAsync(const std::string& name, std::function<void()> onComplete) {
    threadPool.enqueue([this, name, onComplete]() {
        this->loadScene(name);
        if (onComplete) {
            scheduler.scheduleNextFrame(onComplete);
        }
    });
}

void SceneManager::unloadSceneAsync(const std::string& name, std::function<void()> onComplete) {
    threadPool.enqueue([this, name, onComplete]() {
        this->unloadScene(name);
        if (onComplete) {
            scheduler.scheduleNextFrame(onComplete);
        }
    });
}

void SceneManager::transitionTo(const std::string& name, bool unloadCurrent, float delay) {
    if (transitioning) return;

    if (unloadCurrent && currentScene) {
        currentScene->onUnload();
        currentScene = nullptr;
    }

    // Add delay before loading new scene
    if (delay > 0.0f) {
        scheduler.scheduleTask([this, name]() {
            this->loadScene(name);
        }, delay);
    } else {
        loadScene(name);
    }
}

void SceneManager::transitionToAsync(const std::string& name, bool unloadCurrent, float delay, std::function<void()> onComplete) {
    if (transitioning) return;
    transitioning = true;

    std::string currentName = currentScene ? currentScene->getName() : "";

    threadPool.enqueue([this, name, currentName, unloadCurrent, delay, onComplete]() {
        if (unloadCurrent && !currentName.empty()) {
            this->unloadScene(currentName);
        }

        // Add delay before loading new scene
        if (delay > 0.0f) {
            // Sleep in the thread pool thread
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(delay * 1000)));
        }

        this->loadScene(name);

        scheduler.scheduleNextFrame([this, onComplete]() {
            this->transitioning = false;
            if (onComplete) {
                onComplete();
            }
        });
    });
}

std::shared_ptr<Scene> SceneManager::getScene(const std::string& name) {
    auto it = scenes.find(name);
    return (it != scenes.end()) ? it->second : nullptr;
}

void SceneManager::update(float deltaTime) {
    scheduler.update(deltaTime);

    if (currentScene && currentScene->isLoaded()) {
        currentScene->update(deltaTime);
    }
}

void SceneManager::render() {
    if (currentScene && currentScene->isLoaded()) {
        currentScene->render();
    }
}

bool SceneManager::isSceneLoaded(const std::string& name) const {
    auto it = scenes.find(name);
    return (it != scenes.end() && it->second->isLoaded());
}
