#pragma once

#include "../../engine/scene/IScene.h"
#include "../../engine/scene/Entity.h"
#include "../../engine/scene/Prefab.h"
#include "../../client/network/NetworkClient.h"
#include "../../engine/physics/PhysicsEngine.h"
#include "../../engine/rendering/RenderEngine.h"
#include <vector>
#include <unordered_map>
#include <memory>

/**
 * RaidScene - Active gameplay in a raid instance
 *
 * Responsibilities:
 * - Spawn player, AI, loot, and environment entities
 * - Simulate physics, AI, and combat
 * - Render 3D world with particles, weather, audio
 * - Handle networked gameplay (PLAYER_MOVE, PLAYER_SHOOT, etc.)
 * - Manage extraction and raid completion
 *
 * This is the main gameplay scene - uses all engine subsystems.
 */
class RaidScene : public IScene {
public:
    RaidScene(NetworkClient* netClient, uint64_t playerAccId);
    ~RaidScene() override;

    std::string name() const override { return "Raid"; }

    bool onEnter() override;
    void onExit() override;

    void fixedUpdate(float dt) override;
    void update(float dt) override;
    void render() override;

    // Entity management
    EntityId spawnEntity(const Prefab& prefab, const Transform& transform);
    void destroyEntity(EntityId eid);
    Entity* getEntity(EntityId eid);

    // Input handling
    void handleInput(char key);
    void handleMouseClick(float x, float y);
    void handleMouseMove(float x, float y);

private:
    void loadMap();
    void spawnPlayer();
    void spawnAI();
    void spawnLoot();

    void processNetworkPackets();
    void updatePlayer(float dt);
    void updateAI(float dt);
    void updatePhysics(float dt);

    NetworkClient* networkClient;
    uint64_t playerAccountId;

    // Entity tracking
    std::unordered_map<EntityId, Entity> entities;
    std::unordered_map<std::string, Prefab> prefabs;
    EntityId nextEntityId;
    EntityId playerEntityId;

    // Camera
    Vec3 cameraPosition;
    Vec3 cameraRotation;

    // Input state
    float mouseX, mouseY;
    bool moveForward, moveBackward, moveLeft, moveRight;
};
