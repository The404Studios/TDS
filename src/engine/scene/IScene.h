#pragma once

#include <string>

/**
 * IScene - Base interface for all game scenes
 *
 * Scenes are self-contained gameplay states (Menu, Raid, Hideout, etc.)
 * Each scene owns its instances, manages its lifecycle, and decides what to load/unload.
 * Only ONE scene is active at a time - the SceneManager handles switching.
 */
class IScene {
public:
    virtual ~IScene() = default;

    /**
     * Scene identifier (e.g., "Menu", "Raid", "Hideout")
     */
    virtual std::string name() const = 0;

    /**
     * Called when scene becomes active
     * Load content, register network callbacks, spawn initial entities
     * @return true if successful, false to abort scene switch
     */
    virtual bool onEnter() = 0;

    /**
     * Called when scene is being deactivated
     * Unload content, unregister callbacks, cleanup instances
     */
    virtual void onExit() = 0;

    /**
     * Fixed timestep update (for physics, networking)
     * Called at consistent intervals (e.g., 60Hz) regardless of frame rate
     * @param dt Fixed delta time (e.g., 1/60 = 0.0166s)
     */
    virtual void fixedUpdate(float dt) = 0;

    /**
     * Variable timestep update (for game logic, animations, input)
     * Called once per frame
     * @param dt Frame delta time
     */
    virtual void update(float dt) = 0;

    /**
     * Render the scene
     * Called after update, renders all visual content
     */
    virtual void render() = 0;
};
