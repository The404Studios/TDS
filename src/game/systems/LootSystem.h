#pragma once

#include "../../common/DataStructures.h"
#include "../../common/ItemDatabase.h"
#include "../world/BuildingSystem.h"
#include <vector>
#include <memory>
#include <string>
#include <map>

/**
 * LootTableEntry - Single entry in a loot table
 */
struct LootTableEntry {
    std::string itemId;
    float spawnChance;    // 0.0 - 1.0
    int minQuantity;
    int maxQuantity;

    LootTableEntry() : spawnChance(1.0f), minQuantity(1), maxQuantity(1) {}
    LootTableEntry(const std::string& id, float chance, int minQty = 1, int maxQty = 1)
        : itemId(id), spawnChance(chance), minQuantity(minQty), maxQuantity(maxQty) {}
};

/**
 * LootTable - Collection of items that can spawn
 */
struct LootTable {
    std::string name;
    std::vector<LootTableEntry> entries;
    int minItemsToSpawn;
    int maxItemsToSpawn;

    LootTable() : minItemsToSpawn(1), maxItemsToSpawn(3) {}

    void addEntry(const LootTableEntry& entry) {
        entries.push_back(entry);
    }

    void addEntry(const std::string& itemId, float chance, int minQty = 1, int maxQty = 1) {
        entries.push_back(LootTableEntry(itemId, chance, minQty, maxQty));
    }
};

/**
 * LootContainer - Container that holds loot (crate, safe, etc.)
 */
class LootContainer {
public:
    enum class ContainerType {
        CRATE,
        SAFE,
        CABINET,
        DESK,
        CORPSE,
        WEAPON_RACK,
        AMMO_BOX,
        MEDICAL_CASE,
        FOOD_CRATE
    };

    LootContainer(ContainerType type, const Vec3& position);
    ~LootContainer() = default;

    // Loot generation
    void generateLoot(const LootTable& table);
    void addItem(const Item& item);
    void removeItem(uint32_t instanceId);

    // Interaction
    bool isLocked() const { return locked; }
    void unlock() { locked = false; }
    bool isOpened() const { return opened; }
    void open() { opened = true; }
    bool isEmpty() const { return items.empty(); }

    // Getters
    ContainerType getType() const { return type; }
    Vec3 getPosition() const { return position; }
    const std::vector<Item>& getItems() const { return items; }
    std::vector<Item>& getItems() { return items; }

private:
    ContainerType type;
    Vec3 position;
    bool locked;
    bool opened;
    int requiredKey;  // 0 = no key needed
    std::vector<Item> items;
};

/**
 * LootSpawnPoint - Location where loot can spawn
 */
struct LootSpawnPoint {
    Vec3 position;
    std::string lootTableId;
    bool hasSpawned;
    bool isContainer;
    LootContainer* container;  // If isContainer = true

    LootSpawnPoint() : hasSpawned(false), isContainer(false), container(nullptr) {}
};

/**
 * LootZone - Area with specific loot characteristics
 */
struct LootZone {
    enum class ZoneType {
        RESIDENTIAL,   // Houses, apartments
        COMMERCIAL,    // Stores, shops
        INDUSTRIAL,    // Warehouses, factories
        MILITARY,      // Military bases
        MEDICAL,       // Hospitals
        HIGH_VALUE     // Special loot areas
    };

    ZoneType type;
    Vec3 center;
    float radius;
    std::string lootTableId;
    float lootDensity;  // Spawn points per square meter

    LootZone() : type(ZoneType::RESIDENTIAL), radius(50.0f), lootDensity(0.1f) {}
};

/**
 * LootSystem - Manages loot spawning and containers
 */
class LootSystem {
public:
    LootSystem();
    ~LootSystem();

    // Initialization
    void initialize();
    void shutdown();

    // Loot tables
    void registerLootTable(const std::string& id, const LootTable& table);
    const LootTable* getLootTable(const std::string& id) const;

    // Spawn loot
    void spawnLootInBuilding(Building* building);
    void spawnLootInZone(const LootZone& zone);
    void spawnLootAtPoint(LootSpawnPoint& spawnPoint);

    // Containers
    LootContainer* createContainer(LootContainer::ContainerType type, const Vec3& position);
    void removeContainer(LootContainer* container);
    LootContainer* getContainerAt(const Vec3& position, float maxDistance = 2.0f);

    // Loot zones
    void addLootZone(const LootZone& zone);
    void generateLootInAllZones();

    // Update
    void update(float deltaTime);

    // Queries
    std::vector<LootContainer*> getContainersInRadius(const Vec3& position, float radius);
    int getTotalLootItems() const;

    // Clear
    void clearAllLoot();
    void clearLootInRadius(const Vec3& position, float radius);

    // Getters
    const std::vector<std::unique_ptr<LootContainer>>& getContainers() const { return containers; }
    const std::vector<LootSpawnPoint>& getSpawnPoints() const { return spawnPoints; }

private:
    std::map<std::string, LootTable> lootTables;
    std::vector<std::unique_ptr<LootContainer>> containers;
    std::vector<LootSpawnPoint> spawnPoints;
    std::vector<LootZone> lootZones;

    // Helpers
    void initializeDefaultLootTables();
    Item generateItemFromEntry(const LootTableEntry& entry);
    int rollQuantity(int min, int max);
    bool rollChance(float chance);
    std::string selectLootTableForRoom(Room::RoomType roomType);
};
