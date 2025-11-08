#include "SceneManager.h"
#include <iostream>

SceneManager::SceneManager()
    : active(nullptr), accumulator(0.0f) {
}

SceneManager::~SceneManager() {
    unloadAll();
}

bool SceneManager::switchTo(const std::string& key) {
    if (scenes.find(key) == scenes.end()) {
        std::cerr << "[SceneManager] Scene '" << key << "' not found!" << std::endl;
        return false;
    }

    std::cout << "[SceneManager] Scheduling switch to: " << key << std::endl;
    pendingSwitch = key;
    return true;
}

void SceneManager::tick(float dt, float fixedDt) {
    // Handle pending scene switch at safe point (start of frame)
    if (!pendingSwitch.empty()) {
        doSwitch();
    }

    if (!active) {
        return;  // No active scene
    }

    // Fixed timestep update (physics, networking)
    accumulator += dt;
    while (accumulator >= fixedDt) {
        active->fixedUpdate(fixedDt);
        accumulator -= fixedDt;
    }

    // Variable timestep update (game logic)
    active->update(dt);

    // Render
    active->render();
}

void SceneManager::doSwitch() {
    // Exit current scene
    if (active) {
        std::cout << "[SceneManager] Exiting scene: " << active->name() << std::endl;
        active->onExit();
        active = nullptr;
    }

    // Enter new scene
    auto it = scenes.find(pendingSwitch);
    if (it != scenes.end()) {
        active = it->second.get();
        std::cout << "[SceneManager] Entering scene: " << active->name() << std::endl;

        if (!active->onEnter()) {
            std::cerr << "[SceneManager] Scene failed to initialize!" << std::endl;
            active = nullptr;
        }
    }

    pendingSwitch.clear();
    accumulator = 0.0f;  // Reset accumulator on scene switch
}

void SceneManager::unloadAll() {
    if (active) {
        active->onExit();
        active = nullptr;
    }
    scenes.clear();
}
