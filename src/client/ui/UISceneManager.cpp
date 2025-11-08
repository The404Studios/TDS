#include "UISceneManager.h"

UISceneManager::UISceneManager()
    : currentScene(nullptr), threadPool(4), transitioning(false) {
}

UISceneManager::~UISceneManager() {
    unloadAllScenes();
}

void UISceneManager::registerScene(const std::string& name, std::shared_ptr<Scene> scene) {
    scenes[name] = scene;
}

void UISceneManager::unregisterScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it != scenes.end()) {
        if (it->second->isLoaded()) {
            it->second->onUnload();
        }
        scenes.erase(it);
    }
}

void UISceneManager::loadScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it != scenes.end() && !it->second->isLoaded()) {
        it->second->onLoad();
        currentScene = it->second;
    }
}

void UISceneManager::unloadScene(const std::string& name) {
    auto it = scenes.find(name);
    if (it != scenes.end() && it->second->isLoaded()) {
        it->second->onUnload();
        if (currentScene == it->second) {
            currentScene = nullptr;
        }
    }
}

void UISceneManager::unloadAllScenes() {
    for (auto& pair : scenes) {
        if (pair.second->isLoaded()) {
            pair.second->onUnload();
        }
    }
    currentScene = nullptr;
}

void UISceneManager::loadSceneAsync(const std::string& name, std::function<void()> onComplete) {
    threadPool.enqueue([this, name, onComplete]() {
        this->loadScene(name);
        if (onComplete) {
            scheduler.scheduleNextFrame(onComplete);
        }
    });
}

void UISceneManager::unloadSceneAsync(const std::string& name, std::function<void()> onComplete) {
    threadPool.enqueue([this, name, onComplete]() {
        this->unloadScene(name);
        if (onComplete) {
            scheduler.scheduleNextFrame(onComplete);
        }
    });
}

void UISceneManager::transitionTo(const std::string& name, bool unloadCurrent) {
    if (transitioning) return;

    if (unloadCurrent && currentScene) {
        currentScene->onUnload();
        currentScene = nullptr;
    }

    loadScene(name);
}

void UISceneManager::transitionToAsync(const std::string& name, bool unloadCurrent, std::function<void()> onComplete) {
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

std::shared_ptr<Scene> UISceneManager::getScene(const std::string& name) {
    auto it = scenes.find(name);
    return (it != scenes.end()) ? it->second : nullptr;
}

void UISceneManager::update(float deltaTime) {
    scheduler.update(deltaTime);

    if (currentScene && currentScene->isLoaded()) {
        currentScene->update(deltaTime);
    }
}

void UISceneManager::render() {
    if (currentScene && currentScene->isLoaded()) {
        currentScene->render();
    }
}

bool UISceneManager::isSceneLoaded(const std::string& name) const {
    auto it = scenes.find(name);
    return (it != scenes.end() && it->second->isLoaded());
}
