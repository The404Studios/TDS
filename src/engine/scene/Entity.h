#pragma once

#include "../rendering/RenderEngine.h"
#include <cstdint>
#include <string>

/**
 * EntityId - Unique identifier for each entity instance
 */
using EntityId = uint32_t;
constexpr EntityId INVALID_ENTITY = 0;

/**
 * Transform - Position, rotation, and scale in 3D space
 */
struct Transform {
    Vec3 position;
    Vec3 rotation;  // Euler angles (yaw, pitch, roll)
    Vec3 scale;

    Transform()
        : position(0.0f, 0.0f, 0.0f),
          rotation(0.0f, 0.0f, 0.0f),
          scale(1.0f, 1.0f, 1.0f) {}

    Transform(const Vec3& pos, const Vec3& rot = Vec3(), const Vec3& scl = Vec3(1, 1, 1))
        : position(pos), rotation(rot), scale(scl) {}
};

/**
 * EntityFlags - Bitfield for entity state
 */
enum EntityFlags : uint32_t {
    ENTITY_ACTIVE       = 1 << 0,  // Entity is active and updating
    ENTITY_VISIBLE      = 1 << 1,  // Entity is visible and rendering
    ENTITY_PHYSICS      = 1 << 2,  // Entity has physics simulation
    ENTITY_NETWORKED    = 1 << 3,  // Entity is networked (replicated)
    ENTITY_PLAYER       = 1 << 4,  // Entity is a player
    ENTITY_AI           = 1 << 5,  // Entity has AI
    ENTITY_LOOTABLE     = 1 << 6,  // Entity can be looted
    ENTITY_DESTRUCTIBLE = 1 << 7,  // Entity can be destroyed
};

/**
 * Entity - Minimal runtime instance
 *
 * An entity is a live instance in the scene. It has an ID, transform, and
 * references to its prefab definition. Additional components are stored
 * in separate arrays for cache-friendly iteration.
 */
struct Entity {
    EntityId id;
    Transform transform;
    std::string prefabId;  // References ItemDatabase or scene-local prefabs
    uint32_t flags;

    Entity()
        : id(INVALID_ENTITY),
          flags(ENTITY_ACTIVE | ENTITY_VISIBLE) {}

    Entity(EntityId eid, const std::string& prefab)
        : id(eid),
          prefabId(prefab),
          flags(ENTITY_ACTIVE | ENTITY_VISIBLE) {}

    bool isActive() const { return (flags & ENTITY_ACTIVE) != 0; }
    bool isVisible() const { return (flags & ENTITY_VISIBLE) != 0; }
    bool hasPhysics() const { return (flags & ENTITY_PHYSICS) != 0; }
    bool isNetworked() const { return (flags & ENTITY_NETWORKED) != 0; }
};
