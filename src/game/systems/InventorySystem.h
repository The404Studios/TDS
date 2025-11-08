#pragma once

#include "../../common/DataStructures.h"
#include "../../common/ItemDatabase.h"
#include <vector>
#include <memory>
#include <functional>

/**
 * InventoryGrid - Grid-based inventory (like Tarkov/Resident Evil)
 * Items take up space based on width x height
 */
class InventoryGrid {
public:
    InventoryGrid(int width, int height);
    ~InventoryGrid() = default;

    // Item placement
    bool canPlaceItem(const Item& item, int x, int y) const;
    bool placeItem(const Item& item, int x, int y);
    bool removeItem(uint32_t instanceId);
    bool moveItem(uint32_t instanceId, int newX, int newY);

    // Item queries
    Item* getItemAt(int x, int y);
    Item* getItemByInstanceId(uint32_t instanceId);
    std::vector<Item*> getAllItems();

    // Auto-placement (finds first available spot)
    bool autoPlaceItem(const Item& item, int& outX, int& outY);
    bool addItem(const Item& item);  // Convenience function

    // Grid info
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getUsedSlots() const;
    int getTotalSlots() const { return width * height; }
    float getUsagePercent() const;

    // Clear
    void clear();

    // Serialization
    void saveToPlayerData(std::vector<Item>& outItems) const;
    void loadFromPlayerData(const std::vector<Item>& items);

private:
    struct GridSlot {
        uint32_t itemInstanceId;  // 0 = empty, otherwise points to item
        GridSlot() : itemInstanceId(0) {}
    };

    int width;
    int height;
    std::vector<std::vector<GridSlot>> grid;  // [y][x]
    std::vector<Item> items;  // Actual item storage
    uint32_t nextInstanceId;
};

/**
 * Inventory - Complete player inventory system
 * Includes equipped gear, backpack, pockets
 */
class Inventory {
public:
    Inventory();
    ~Inventory() = default;

    // Grids
    InventoryGrid* getStash() { return stash.get(); }
    InventoryGrid* getBackpack() { return backpack.get(); }
    InventoryGrid* getPockets() { return pockets.get(); }

    // Equipped items
    bool equipItem(const Item& item, const std::string& slot);
    bool unequipItem(const std::string& slot, Item& outItem);
    Item* getEquippedItem(const std::string& slot);

    // Equipment slots
    std::vector<std::string> getEquipmentSlots() const {
        return {"helmet", "armor", "backpack", "weapon_primary",
                "weapon_secondary", "weapon_melee"};
    }

    // Quick access
    bool hasItem(const std::string& itemId) const;
    int getItemCount(const std::string& itemId) const;
    bool consumeItem(const std::string& itemId, int amount = 1);

    // Weight/Value
    float getTotalWeight() const;
    int getTotalValue() const;

    // Callbacks for UI updates
    void setOnInventoryChanged(std::function<void()> callback) { onInventoryChanged = callback; }

private:
    void notifyChanged();

    // Grid storage
    std::unique_ptr<InventoryGrid> stash;      // Persistent storage (10x40)
    std::unique_ptr<InventoryGrid> backpack;   // In-raid storage (varies by backpack)
    std::unique_ptr<InventoryGrid> pockets;    // Quick access (4x2)

    // Equipped items
    std::map<std::string, Item> equipped;

    // Callbacks
    std::function<void()> onInventoryChanged;
};

/**
 * InventoryTransaction - For merchant trades
 */
struct InventoryTransaction {
    std::vector<Item> itemsToGive;
    std::vector<Item> itemsToReceive;
    int roubleChange;  // Positive = receive, negative = pay

    bool isValid() const {
        return !itemsToGive.empty() || !itemsToReceive.empty() || roubleChange != 0;
    }
};
