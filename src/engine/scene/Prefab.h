#pragma once

#include "Entity.h"
#include "../rendering/RenderEngine.h"
#include "../physics/PhysicsEngine.h"
#include "../../common/ItemDatabase.h"
#include <string>
#include <memory>

/**
 * Prefab - Template for spawning entities
 *
 * A prefab is a reusable template that defines what an entity looks like,
 * how it behaves, and what components it has. Prefabs can come from:
 * - ItemDatabase (weapons, armor, loot)
 * - Scene-specific definitions (map objects, NPCs)
 * - Runtime-generated (particles, projectiles)
 */
struct Prefab {
    std::string id;           // Unique identifier (e.g., "ak74", "player", "loot_crate")
    std::string displayName;  // Human-readable name

    // Visual
    std::string meshPath;     // Path to 3D model
    Vec3 meshScale;           // Scale multiplier

    // Physics (optional)
    bool hasPhysics;
    ColliderType colliderType;
    Vec3 colliderSize;
    float mass;

    // Gameplay
    float health;
    float maxHealth;

    Prefab()
        : meshScale(1, 1, 1),
          hasPhysics(false),
          colliderType(ColliderType::BOX),
          colliderSize(1, 1, 1),
          mass(1.0f),
          health(100.0f),
          maxHealth(100.0f) {}

    /**
     * Create prefab from ItemDatabase entry
     * Weapons, armor, and loot use centralized definitions
     */
    static Prefab fromItem(const Item& item) {
        Prefab prefab;
        prefab.id = item.id;
        prefab.displayName = item.name;

        // Map item type to mesh/physics
        switch (item.type) {
            case ItemType::WEAPON:
                prefab.meshPath = "assets/models/weapons/" + item.id + ".obj";
                prefab.hasPhysics = true;
                prefab.colliderType = ColliderType::BOX;
                prefab.colliderSize = Vec3(0.1f, 0.1f, 0.5f);
                prefab.mass = 3.0f;
                break;

            case ItemType::ARMOR:
                prefab.meshPath = "assets/models/armor/" + item.id + ".obj";
                prefab.hasPhysics = false;
                break;

            case ItemType::MEDICAL:
            case ItemType::CONSUMABLE:
                prefab.meshPath = "assets/models/items/" + item.id + ".obj";
                prefab.hasPhysics = true;
                prefab.colliderType = ColliderType::BOX;
                prefab.colliderSize = Vec3(0.05f, 0.05f, 0.1f);
                prefab.mass = 0.2f;
                break;

            case ItemType::AMMO:
                prefab.meshPath = "assets/models/ammo/" + item.id + ".obj";
                prefab.hasPhysics = true;
                prefab.colliderType = ColliderType::BOX;
                prefab.colliderSize = Vec3(0.03f, 0.03f, 0.05f);
                prefab.mass = 0.1f;
                break;

            default:
                prefab.meshPath = "assets/models/items/default.obj";
                break;
        }

        return prefab;
    }

    /**
     * Create player prefab
     */
    static Prefab player() {
        Prefab prefab;
        prefab.id = "player";
        prefab.displayName = "Player";
        prefab.meshPath = "assets/models/player/pmc.obj";
        prefab.hasPhysics = true;
        prefab.colliderType = ColliderType::CAPSULE;
        prefab.colliderSize = Vec3(0.5f, 1.8f, 0.5f);  // radius, height
        prefab.mass = 80.0f;
        prefab.health = 100.0f;
        prefab.maxHealth = 100.0f;
        return prefab;
    }

    /**
     * Create AI scav prefab
     */
    static Prefab scav() {
        Prefab prefab;
        prefab.id = "scav";
        prefab.displayName = "Scav";
        prefab.meshPath = "assets/models/ai/scav.obj";
        prefab.hasPhysics = true;
        prefab.colliderType = ColliderType::CAPSULE;
        prefab.colliderSize = Vec3(0.5f, 1.8f, 0.5f);
        prefab.mass = 75.0f;
        prefab.health = 80.0f;
        prefab.maxHealth = 80.0f;
        return prefab;
    }

    /**
     * Create loot container prefab
     */
    static Prefab lootCrate() {
        Prefab prefab;
        prefab.id = "loot_crate";
        prefab.displayName = "Loot Crate";
        prefab.meshPath = "assets/models/world/crate.obj";
        prefab.hasPhysics = true;
        prefab.colliderType = ColliderType::BOX;
        prefab.colliderSize = Vec3(0.5f, 0.5f, 0.5f);
        prefab.mass = 50.0f;
        prefab.health = 50.0f;
        prefab.maxHealth = 50.0f;
        return prefab;
    }
};
