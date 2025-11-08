#include "LootSystem.h"
#include <algorithm>
#include <random>
#include <cmath>
#include <iostream>

// Random number generator
static std::random_device rd;
static std::mt19937 gen(rd());

// ========== LootContainer Implementation ==========

LootContainer::LootContainer(ContainerType type, const Vec3& position)
    : type(type), position(position), locked(false), opened(false), requiredKey(0) {

    // Some containers start locked
    switch (type) {
        case ContainerType::SAFE:
            locked = true;
            requiredKey = 1;
            break;
        default:
            break;
    }
}

void LootContainer::generateLoot(const LootTable& table) {
    items.clear();

    std::uniform_int_distribution<int> itemCountDist(table.minItemsToSpawn, table.maxItemsToSpawn);
    int numItems = itemCountDist(gen);

    ItemDatabase& itemDb = ItemDatabase::getInstance();

    for (int i = 0; i < numItems; i++) {
        // Randomly select an entry based on spawn chance
        std::uniform_real_distribution<float> chanceDist(0.0f, 1.0f);

        for (const auto& entry : table.entries) {
            if (chanceDist(gen) <= entry.spawnChance) {
                Item item = itemDb.getItem(entry.itemId);
                if (!item.id.empty()) {
                    // Randomize quantity
                    std::uniform_int_distribution<int> qtyDist(entry.minQuantity, entry.maxQuantity);
                    item.stackSize = qtyDist(gen);

                    addItem(item);
                }
                break;  // Only spawn one item per roll
            }
        }
    }

    std::cout << "[LootContainer] Generated " << items.size() << " items" << std::endl;
}

void LootContainer::addItem(const Item& item) {
    items.push_back(item);
}

void LootContainer::removeItem(uint32_t instanceId) {
    auto it = std::remove_if(items.begin(), items.end(),
        [instanceId](const Item& item) {
            return item.instanceId == instanceId;
        });

    if (it != items.end()) {
        items.erase(it, items.end());
    }
}

// ========== LootSystem Implementation ==========

LootSystem::LootSystem() {
}

LootSystem::~LootSystem() {
    shutdown();
}

void LootSystem::initialize() {
    initializeDefaultLootTables();
    std::cout << "[LootSystem] Initialized with " << lootTables.size() << " loot tables" << std::endl;
}

void LootSystem::shutdown() {
    containers.clear();
    spawnPoints.clear();
    lootZones.clear();
    std::cout << "[LootSystem] Shutdown complete" << std::endl;
}

void LootSystem::registerLootTable(const std::string& id, const LootTable& table) {
    lootTables[id] = table;
}

const LootTable* LootSystem::getLootTable(const std::string& id) const {
    auto it = lootTables.find(id);
    if (it != lootTables.end()) {
        return &(it->second);
    }
    return nullptr;
}

void LootSystem::spawnLootInBuilding(Building* building) {
    if (!building) return;

    // Get loot spawn points from building
    auto lootPoints = building->getAllLootSpawnPoints();

    std::cout << "[LootSystem] Spawning loot in building with " << lootPoints.size() << " spawn points" << std::endl;

    for (const Vec3& point : lootPoints) {
        // Create spawn point
        LootSpawnPoint spawnPoint;
        spawnPoint.position = point;

        // Determine loot table based on building and room type
        // For now, use a general loot table
        spawnPoint.lootTableId = "general";

        // 50% chance to spawn a container, 50% chance to spawn loose loot
        std::uniform_real_distribution<float> containerChance(0.0f, 1.0f);
        if (containerChance(gen) < 0.5f) {
            // Create container
            spawnPoint.isContainer = true;

            // Random container type
            std::uniform_int_distribution<int> typeDist(0, 3);
            LootContainer::ContainerType containerType = static_cast<LootContainer::ContainerType>(typeDist(gen));

            LootContainer* container = createContainer(containerType, point);
            spawnPoint.container = container;

            // Generate loot in container
            const LootTable* table = getLootTable(spawnPoint.lootTableId);
            if (table) {
                container->generateLoot(*table);
            }
        }
        else {
            // Spawn loose loot
            spawnLootAtPoint(spawnPoint);
        }

        spawnPoints.push_back(spawnPoint);
    }
}

void LootSystem::spawnLootInZone(const LootZone& zone) {
    // Calculate number of spawn points based on zone area and density
    float area = 3.14159f * zone.radius * zone.radius;
    int numSpawnPoints = static_cast<int>(area * zone.lootDensity);

    std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f);
    std::uniform_real_distribution<float> radiusDist(0.0f, zone.radius);

    for (int i = 0; i < numSpawnPoints; i++) {
        // Random position in zone
        float angle = angleDist(gen);
        float r = radiusDist(gen);

        LootSpawnPoint spawnPoint;
        spawnPoint.position.x = zone.center.x + r * std::cos(angle);
        spawnPoint.position.y = zone.center.y;
        spawnPoint.position.z = zone.center.z + r * std::sin(angle);
        spawnPoint.lootTableId = zone.lootTableId;

        spawnLootAtPoint(spawnPoint);
        spawnPoints.push_back(spawnPoint);
    }

    std::cout << "[LootSystem] Spawned " << numSpawnPoints << " loot points in zone" << std::endl;
}

void LootSystem::spawnLootAtPoint(LootSpawnPoint& spawnPoint) {
    if (spawnPoint.hasSpawned) return;

    const LootTable* table = getLootTable(spawnPoint.lootTableId);
    if (!table) {
        std::cerr << "[LootSystem] Loot table not found: " << spawnPoint.lootTableId << std::endl;
        return;
    }

    // Generate items at this point
    // In a real implementation, items would be spawned as entities in the world
    // For now, we just mark it as spawned

    spawnPoint.hasSpawned = true;
}

LootContainer* LootSystem::createContainer(LootContainer::ContainerType type, const Vec3& position) {
    auto container = std::make_unique<LootContainer>(type, position);
    LootContainer* ptr = container.get();
    containers.push_back(std::move(container));
    return ptr;
}

void LootSystem::removeContainer(LootContainer* container) {
    auto it = std::remove_if(containers.begin(), containers.end(),
        [container](const std::unique_ptr<LootContainer>& c) {
            return c.get() == container;
        });

    if (it != containers.end()) {
        containers.erase(it, containers.end());
    }
}

LootContainer* LootSystem::getContainerAt(const Vec3& position, float maxDistance) {
    float maxDistSq = maxDistance * maxDistance;

    for (auto& container : containers) {
        Vec3 cPos = container->getPosition();
        float dx = cPos.x - position.x;
        float dy = cPos.y - position.y;
        float dz = cPos.z - position.z;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq <= maxDistSq) {
            return container.get();
        }
    }

    return nullptr;
}

void LootSystem::addLootZone(const LootZone& zone) {
    lootZones.push_back(zone);
}

void LootSystem::generateLootInAllZones() {
    for (const auto& zone : lootZones) {
        spawnLootInZone(zone);
    }
}

void LootSystem::update(float deltaTime) {
    // Update container states, respawn timers, etc.
}

std::vector<LootContainer*> LootSystem::getContainersInRadius(const Vec3& position, float radius) {
    std::vector<LootContainer*> result;
    float radiusSq = radius * radius;

    for (auto& container : containers) {
        Vec3 cPos = container->getPosition();
        float dx = cPos.x - position.x;
        float dz = cPos.z - position.z;
        float distSq = dx * dx + dz * dz;

        if (distSq <= radiusSq) {
            result.push_back(container.get());
        }
    }

    return result;
}

int LootSystem::getTotalLootItems() const {
    int total = 0;
    for (const auto& container : containers) {
        total += static_cast<int>(container->getItems().size());
    }
    return total;
}

void LootSystem::clearAllLoot() {
    containers.clear();
    spawnPoints.clear();
}

void LootSystem::clearLootInRadius(const Vec3& position, float radius) {
    float radiusSq = radius * radius;

    // Remove containers
    auto it = std::remove_if(containers.begin(), containers.end(),
        [&position, radiusSq](const std::unique_ptr<LootContainer>& container) {
            Vec3 cPos = container->getPosition();
            float dx = cPos.x - position.x;
            float dz = cPos.z - position.z;
            float distSq = dx * dx + dz * dz;
            return distSq <= radiusSq;
        });

    containers.erase(it, containers.end());

    // Remove spawn points
    auto it2 = std::remove_if(spawnPoints.begin(), spawnPoints.end(),
        [&position, radiusSq](const LootSpawnPoint& spawnPoint) {
            float dx = spawnPoint.position.x - position.x;
            float dz = spawnPoint.position.z - position.z;
            float distSq = dx * dx + dz * dz;
            return distSq <= radiusSq;
        });

    spawnPoints.erase(it2, spawnPoints.end());
}

// Private methods

void LootSystem::initializeDefaultLootTables() {
    ItemDatabase& itemDb = ItemDatabase::getInstance();

    // General loot table
    LootTable general;
    general.name = "general";
    general.minItemsToSpawn = 1;
    general.maxItemsToSpawn = 3;

    // Get all items and add them with appropriate chances
    auto allItems = itemDb.getAllItems();
    for (const Item& item : allItems) {
        float chance = 0.3f;  // 30% base chance

        // Adjust chance by rarity
        switch (item.rarity) {
            case ItemRarity::LEGENDARY:
                chance = 0.05f;  // 5%
                break;
            case ItemRarity::RARE:
                chance = 0.15f;  // 15%
                break;
            case ItemRarity::UNCOMMON:
                chance = 0.25f;  // 25%
                break;
            default:
                chance = 0.4f;   // 40%
                break;
        }

        general.addEntry(item.id, chance);
    }

    registerLootTable("general", general);

    // Weapon loot table (for military areas)
    LootTable weapons;
    weapons.name = "weapons";
    weapons.minItemsToSpawn = 1;
    weapons.maxItemsToSpawn = 2;

    for (const Item& item : allItems) {
        if (item.type == ItemType::WEAPON) {
            float chance = (item.rarity == ItemRarity::LEGENDARY) ? 0.1f : 0.4f;
            weapons.addEntry(item.id, chance);
        }
    }

    registerLootTable("weapons", weapons);

    // Medical loot table
    LootTable medical;
    medical.name = "medical";
    medical.minItemsToSpawn = 2;
    medical.maxItemsToSpawn = 5;

    for (const Item& item : allItems) {
        if (item.type == ItemType::MEDICAL) {
            medical.addEntry(item.id, 0.6f);
        }
    }

    registerLootTable("medical", medical);

    // Food loot table
    LootTable food;
    food.name = "food";
    food.minItemsToSpawn = 2;
    food.maxItemsToSpawn = 4;

    for (const Item& item : allItems) {
        if (item.type == ItemType::FOOD) {
            food.addEntry(item.id, 0.5f, 1, 3);
        }
    }

    registerLootTable("food", food);

    // Ammo loot table
    LootTable ammo;
    ammo.name = "ammo";
    ammo.minItemsToSpawn = 1;
    ammo.maxItemsToSpawn = 3;

    for (const Item& item : allItems) {
        if (item.type == ItemType::AMMO) {
            ammo.addEntry(item.id, 0.4f, 30, 120);  // Ammo spawns in larger quantities
        }
    }

    registerLootTable("ammo", ammo);

    // High value loot table
    LootTable highValue;
    highValue.name = "high_value";
    highValue.minItemsToSpawn = 1;
    highValue.maxItemsToSpawn = 2;

    for (const Item& item : allItems) {
        if (item.rarity == ItemRarity::LEGENDARY || item.rarity == ItemRarity::RARE) {
            float chance = (item.rarity == ItemRarity::LEGENDARY) ? 0.2f : 0.4f;
            highValue.addEntry(item.id, chance);
        }
    }

    registerLootTable("high_value", highValue);

    std::cout << "[LootSystem] Initialized " << lootTables.size() << " default loot tables" << std::endl;
}

Item LootSystem::generateItemFromEntry(const LootTableEntry& entry) {
    ItemDatabase& itemDb = ItemDatabase::getInstance();
    Item item = itemDb.getItem(entry.itemId);

    if (!item.id.empty()) {
        item.stackSize = rollQuantity(entry.minQuantity, entry.maxQuantity);
    }

    return item;
}

int LootSystem::rollQuantity(int min, int max) {
    if (min >= max) return min;

    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

bool LootSystem::rollChance(float chance) {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(gen) <= chance;
}

std::string LootSystem::selectLootTableForRoom(Room::RoomType roomType) {
    switch (roomType) {
        case Room::RoomType::OFFICE:
            return "general";
        case Room::RoomType::STORAGE:
            return "general";
        case Room::RoomType::BEDROOM:
            return "general";
        case Room::RoomType::KITCHEN:
            return "food";
        case Room::RoomType::BATHROOM:
            return "medical";
        default:
            return "general";
    }
}
