#include "SceneManager.h"
#include <iostream>

SceneManager::SceneManager()
    : active(nullptr), accumulator(0.0f), transitioning(false), transitionTimer(0.0f), transitionDelay(0.5f) {
}

SceneManager::~SceneManager() {
    unloadAll();
}

bool SceneManager::switchTo(const std::string& key, float delay) {
    if (scenes.find(key) == scenes.end()) {
        std::cerr << "[SceneManager] Scene '" << key << "' not found!" << std::endl;
        return false;
    }

    std::cout << "[SceneManager] Scheduling switch to: " << key << " (delay: " << delay << "s)" << std::endl;
    pendingSwitch = key;
    transitionDelay = delay;
    transitioning = false;
    transitionTimer = 0.0f;
    return true;
}

void SceneManager::tick(float dt, float fixedDt) {
    // Handle pending scene switch with transition delay
    if (!pendingSwitch.empty()) {
        if (!transitioning) {
            // First phase: Exit current scene
            if (active) {
                std::cout << "[SceneManager] Exiting scene: " << active->name() << std::endl;
                active->onExit();
                active = nullptr;
            }
            transitioning = true;
            transitionTimer = 0.0f;
            std::cout << "[SceneManager] Transition pause started (" << transitionDelay << "s)" << std::endl;
        } else {
            // Wait during transition
            transitionTimer += dt;
            if (transitionTimer >= transitionDelay) {
                // Second phase: Enter new scene
                doSwitch();
            }
        }
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
    // Enter new scene (exit already happened during transition)
    auto it = scenes.find(pendingSwitch);
    if (it != scenes.end()) {
        active = it->second.get();
        std::cout << "[SceneManager] Transition complete. Entering scene: " << active->name() << std::endl;

        if (!active->onEnter()) {
            std::cerr << "[SceneManager] Scene failed to initialize!" << std::endl;
            active = nullptr;
        }
    }

    pendingSwitch.clear();
    transitioning = false;
    transitionTimer = 0.0f;
    accumulator = 0.0f;  // Reset accumulator on scene switch
}

void SceneManager::unloadAll() {
    if (active) {
        active->onExit();
        active = nullptr;
    }
    scenes.clear();
}
