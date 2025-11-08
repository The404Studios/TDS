#include "SceneManager.h"
#include <iostream>

SceneManager::SceneManager()
    : active(nullptr), accumulator(0.0f), transitionDelay(0.5f), transitionTimer(0.0f) {
}

SceneManager::~SceneManager() {
    unloadAll();
}

bool SceneManager::switchTo(const std::string& key) {
    if (scenes.find(key) == scenes.end()) {
        std::cerr << "[SceneManager] Scene '" << key << "' not found!" << std::endl;
        return false;
    }

    // Don't allow switching if already transitioning
    if (transitionTimer > 0.0f) {
        std::cout << "[SceneManager] Already transitioning, ignoring request" << std::endl;
        return false;
    }

    std::cout << "[SceneManager] Scheduling switch to: " << key << std::endl;
    pendingSwitch = key;
    return true;
}

void SceneManager::tick(float dt, float fixedDt) {
    // Handle transition timer (pause between scenes)
    if (transitionTimer > 0.0f) {
        transitionTimer -= dt;

        // When transition completes, enter the new scene
        if (transitionTimer <= 0.0f) {
            transitionTimer = 0.0f;
            std::cout << "[SceneManager] Transition delay complete" << std::endl;

            // Enter new scene
            if (!pendingSwitch.empty()) {
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
        }

        // Don't update scenes during transition
        return;
    }

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

        // Start transition delay/pause
        if (transitionDelay > 0.0f) {
            std::cout << "[SceneManager] Starting transition delay: " << transitionDelay << "s" << std::endl;
            transitionTimer = transitionDelay;
            // Scene will be entered after the timer expires in tick()
            return;
        }
    }

    // If no transition delay, enter new scene immediately
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
