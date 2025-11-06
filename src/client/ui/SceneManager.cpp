#include "SceneManager.h"

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
    for (auto& pair : scenes) {
        if (pair.second->isLoaded()) {
            pair.second->onUnload();
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

void SceneManager::transitionTo(const std::string& name, bool unloadCurrent) {
    if (transitioning) return;

    if (unloadCurrent && currentScene) {
        currentScene->onUnload();
        currentScene = nullptr;
    }

    loadScene(name);
}

void SceneManager::transitionToAsync(const std::string& name, bool unloadCurrent, std::function<void()> onComplete) {
    if (transitioning) return;
    transitioning = true;

    std::string currentName = currentScene ? currentScene->getName() : "";

    threadPool.enqueue([this, name, currentName, unloadCurrent, onComplete]() {
        if (unloadCurrent && !currentName.empty()) {
            this->unloadScene(currentName);
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
