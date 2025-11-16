// Corpse and Body Looting System
// Manages dead player bodies and their loot

#pragma once

#include "DataStructures.h"
#include "ItemDatabase.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <chrono>
#include <iostream>

// Represents a corpse in the world
struct Corpse {
    uint64_t corpseId;              // Unique corpse ID
    uint64_t playerId;              // ID of the player who died
    std::string playerName;          // Name of the player who died
    float position[3];               // World position (x, y, z)
    float rotation;                  // Body rotation (yaw)
    std::chrono::steady_clock::time_point deathTime;

    // Loot inventory (items on the body)
    std::vector<Item> inventory;    // All items the player had when they died
    std::vector<Item> equipped;      // Worn equipment (armor, helmet, backpack)

    bool isLooted;                   // Has been looted
    uint64_t lootedBy;               // Player ID who looted (0 if not looted)

    Corpse()
        : corpseId(0)
        , playerId(0)
        , rotation(0.0f)
        , isLooted(false)
        , lootedBy(0)
    {
        position[0] = position[1] = position[2] = 0.0f;
        deathTime = std::chrono::steady_clock::now();
    }

    Corpse(uint64_t id, uint64_t pId, const std::string& name, float x, float y, float z, float rot)
        : corpseId(id)
        , playerId(pId)
        , playerName(name)
        , rotation(rot)
        , isLooted(false)
        , lootedBy(0)
    {
        position[0] = x;
        position[1] = y;
        position[2] = z;
        deathTime = std::chrono::steady_clock::now();
    }

    // Get total item count
    int getTotalItemCount() const {
        return static_cast<int>(inventory.size() + equipped.size());
    }

    // Check if corpse is nearby
    bool isNearby(float x, float y, float z, float maxDistance) const {
        float dx = position[0] - x;
        float dy = position[1] - y;
        float dz = position[2] - z;
        float distSq = dx * dx + dy * dy + dz * dz;
        return distSq <= (maxDistance * maxDistance);
    }

    // Get age in seconds
    int getAgeSeconds() const {
        auto now = std::chrono::steady_clock::now();
        return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(now - deathTime).count());
    }
};

// Server-side corpse manager
class CorpseManager {
private:
    std::unordered_map<uint64_t, Corpse> m_corpses;
    uint64_t m_nextCorpseId;

    static constexpr int CORPSE_LIFETIME_SECONDS = 1800; // 30 minutes
    static constexpr float LOOT_DISTANCE = 3.0f;          // Max distance to loot

public:
    CorpseManager() : m_nextCorpseId(1) {}

    // Create a corpse from a dead player
    uint64_t createCorpse(uint64_t playerId, const std::string& playerName,
                         float x, float y, float z, float rotation,
                         const std::vector<Item>& inventory,
                         const std::vector<Item>& equipped) {

        uint64_t corpseId = m_nextCorpseId++;

        Corpse corpse(corpseId, playerId, playerName, x, y, z, rotation);
        corpse.inventory = inventory;
        corpse.equipped = equipped;

        m_corpses[corpseId] = corpse;

        std::cout << "[CorpseManager] Created corpse " << corpseId
                  << " for player " << playerName
                  << " with " << corpse.getTotalItemCount() << " items" << std::endl;

        return corpseId;
    }

    // Get corpse by ID
    Corpse* getCorpse(uint64_t corpseId) {
        auto it = m_corpses.find(corpseId);
        return (it != m_corpses.end()) ? &it->second : nullptr;
    }

    // Get all corpses in the world
    std::vector<Corpse> getAllCorpses() const {
        std::vector<Corpse> result;
        for (const auto& pair : m_corpses) {
            result.push_back(pair.second);
        }
        return result;
    }

    // Find corpses near a position
    std::vector<uint64_t> findCorpsesNearby(float x, float y, float z, float maxDistance = LOOT_DISTANCE) {
        std::vector<uint64_t> nearby;

        for (const auto& pair : m_corpses) {
            if (pair.second.isNearby(x, y, z, maxDistance)) {
                nearby.push_back(pair.first);
            }
        }

        return nearby;
    }

    // Loot an item from corpse
    bool lootItem(uint64_t corpseId, int itemIndex, uint64_t playerId, bool fromEquipped = false) {
        auto it = m_corpses.find(corpseId);
        if (it == m_corpses.end()) return false;

        Corpse& corpse = it->second;

        std::vector<Item>& source = fromEquipped ? corpse.equipped : corpse.inventory;

        if (itemIndex < 0 || itemIndex >= static_cast<int>(source.size())) {
            return false;
        }

        // Remove item from corpse
        source.erase(source.begin() + itemIndex);

        // Mark as looted
        if (!corpse.isLooted) {
            corpse.isLooted = true;
            corpse.lootedBy = playerId;
        }

        std::cout << "[CorpseManager] Player " << playerId << " looted item from corpse " << corpseId << std::endl;

        return true;
    }

    // Loot all items from corpse
    std::vector<Item> lootAll(uint64_t corpseId, uint64_t playerId) {
        std::vector<Item> lootedItems;

        auto it = m_corpses.find(corpseId);
        if (it == m_corpses.end()) return lootedItems;

        Corpse& corpse = it->second;

        // Collect all items
        lootedItems.insert(lootedItems.end(), corpse.inventory.begin(), corpse.inventory.end());
        lootedItems.insert(lootedItems.end(), corpse.equipped.begin(), corpse.equipped.end());

        // Clear corpse inventory
        corpse.inventory.clear();
        corpse.equipped.clear();

        // Mark as looted
        corpse.isLooted = true;
        corpse.lootedBy = playerId;

        std::cout << "[CorpseManager] Player " << playerId
                  << " looted all " << lootedItems.size()
                  << " items from corpse " << corpseId << std::endl;

        return lootedItems;
    }

    // Remove old corpses
    void cleanupOldCorpses() {
        std::vector<uint64_t> toRemove;

        for (const auto& pair : m_corpses) {
            if (pair.second.getAgeSeconds() > CORPSE_LIFETIME_SECONDS) {
                toRemove.push_back(pair.first);
            }
        }

        for (uint64_t id : toRemove) {
            std::cout << "[CorpseManager] Removing expired corpse " << id << std::endl;
            m_corpses.erase(id);
        }
    }

    // Remove specific corpse
    void removeCorpse(uint64_t corpseId) {
        m_corpses.erase(corpseId);
    }

    // Get corpse count
    int getCorpseCount() const {
        return static_cast<int>(m_corpses.size());
    }

    // Check if player can loot corpse (distance check)
    bool canLootCorpse(uint64_t corpseId, float playerX, float playerY, float playerZ) const {
        auto it = m_corpses.find(corpseId);
        if (it == m_corpses.end()) return false;

        return it->second.isNearby(playerX, playerY, playerZ, LOOT_DISTANCE);
    }
};

// Network packets for corpse system (add to NetworkProtocol.h)
enum class CorpsePacketType : uint16_t {
    CORPSE_SPAWN = 320,         // Server -> Client: Spawn corpse
    CORPSE_REQUEST_LOOT = 321,  // Client -> Server: Request to loot corpse
    CORPSE_LOOT_ITEM = 322,     // Server -> Client: Confirm item looted
    CORPSE_LOOT_ALL = 323,      // Client -> Server: Loot all items
    CORPSE_UPDATE = 324,        // Server -> Client: Update corpse state
    CORPSE_REMOVE = 325         // Server -> Client: Remove corpse
};

#pragma pack(push, 1)

// Server spawns corpse for clients
struct CorpseSpawnPacket {
    uint64_t corpseId;
    uint64_t playerId;
    char playerName[32];
    float position[3];
    float rotation;
    uint32_t itemCount;
    uint32_t equippedCount;
    // Followed by Item array for inventory and equipped
};

// Client requests to loot specific item
struct CorpseLootRequestPacket {
    uint64_t corpseId;
    uint32_t itemIndex;
    bool fromEquipped;
};

// Client requests to loot all
struct CorpseLootAllRequestPacket {
    uint64_t corpseId;
};

// Server confirms loot
struct CorpseLootResponsePacket {
    uint64_t corpseId;
    uint32_t itemIndex;
    bool fromEquipped;
    bool success;
    // Followed by Item data if success
};

// Remove corpse from world
struct CorpseRemovePacket {
    uint64_t corpseId;
};

#pragma pack(pop)

// Client-side corpse representation (for rendering)
struct ClientCorpse {
    uint64_t corpseId;
    std::string playerName;
    float position[3];
    float rotation;
    bool hasLoot;

    ClientCorpse() : corpseId(0), rotation(0.0f), hasLoot(true) {
        position[0] = position[1] = position[2] = 0.0f;
    }
};
