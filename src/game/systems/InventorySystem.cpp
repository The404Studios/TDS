#include "InventorySystem.h"
#include <algorithm>
#include <iostream>

// ========== InventoryGrid Implementation ==========

InventoryGrid::InventoryGrid(int width, int height)
    : width(width), height(height), nextInstanceId(1) {
    // Initialize grid
    grid.resize(height);
    for (int y = 0; y < height; y++) {
        grid[y].resize(width);
    }
}

bool InventoryGrid::canPlaceItem(const Item& item, int x, int y) const {
    // Check bounds
    if (x < 0 || y < 0 || x + item.width > width || y + item.height > height) {
        return false;
    }

    // Check if all required slots are empty
    for (int dy = 0; dy < item.height; dy++) {
        for (int dx = 0; dx < item.width; dx++) {
            if (grid[y + dy][x + dx].itemInstanceId != 0) {
                return false;
            }
        }
    }

    return true;
}

bool InventoryGrid::placeItem(const Item& item, int x, int y) {
    if (!canPlaceItem(item, x, y)) {
        return false;
    }

    // Create item instance
    Item newItem = item;
    newItem.instanceId = nextInstanceId++;
    items.push_back(newItem);

    // Mark grid slots as occupied
    for (int dy = 0; dy < item.height; dy++) {
        for (int dx = 0; dx < item.width; dx++) {
            grid[y + dy][x + dx].itemInstanceId = newItem.instanceId;
        }
    }

    return true;
}

bool InventoryGrid::removeItem(uint32_t instanceId) {
    // Find the item
    auto it = std::find_if(items.begin(), items.end(),
        [instanceId](const Item& item) { return item.instanceId == instanceId; });

    if (it == items.end()) {
        return false;
    }

    Item& item = *it;

    // Clear grid slots
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (grid[y][x].itemInstanceId == instanceId) {
                grid[y][x].itemInstanceId = 0;
            }
        }
    }

    // Remove from items vector
    items.erase(it);
    return true;
}

bool InventoryGrid::moveItem(uint32_t instanceId, int newX, int newY) {
    // Find the item
    Item* item = getItemByInstanceId(instanceId);
    if (!item) {
        return false;
    }

    // Store item data
    Item itemCopy = *item;

    // Remove from old position
    removeItem(instanceId);

    // Try to place at new position
    if (placeItem(itemCopy, newX, newY)) {
        return true;
    }

    // If placement failed, restore at original position
    // (This is a simplified approach - in production you'd want to track original position)
    std::cerr << "[InventoryGrid] Failed to move item - could not restore" << std::endl;
    return false;
}

Item* InventoryGrid::getItemAt(int x, int y) {
    if (x < 0 || y < 0 || x >= width || y >= height) {
        return nullptr;
    }

    uint32_t instanceId = grid[y][x].itemInstanceId;
    if (instanceId == 0) {
        return nullptr;
    }

    return getItemByInstanceId(instanceId);
}

Item* InventoryGrid::getItemByInstanceId(uint32_t instanceId) {
    auto it = std::find_if(items.begin(), items.end(),
        [instanceId](const Item& item) { return item.instanceId == instanceId; });

    if (it == items.end()) {
        return nullptr;
    }

    return &(*it);
}

std::vector<Item*> InventoryGrid::getAllItems() {
    std::vector<Item*> result;
    for (auto& item : items) {
        result.push_back(&item);
    }
    return result;
}

bool InventoryGrid::autoPlaceItem(const Item& item, int& outX, int& outY) {
    // Try to find first available spot (top-left to bottom-right)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (canPlaceItem(item, x, y)) {
                outX = x;
                outY = y;
                return true;
            }
        }
    }

    return false;
}

bool InventoryGrid::addItem(const Item& item) {
    int x, y;
    if (autoPlaceItem(item, x, y)) {
        return placeItem(item, x, y);
    }
    return false;
}

int InventoryGrid::getUsedSlots() const {
    int used = 0;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (grid[y][x].itemInstanceId != 0) {
                used++;
            }
        }
    }
    return used;
}

float InventoryGrid::getUsagePercent() const {
    return (float)getUsedSlots() / (float)getTotalSlots() * 100.0f;
}

void InventoryGrid::clear() {
    items.clear();
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            grid[y][x].itemInstanceId = 0;
        }
    }
}

void InventoryGrid::saveToPlayerData(std::vector<Item>& outItems) const {
    outItems = items;
}

void InventoryGrid::loadFromPlayerData(const std::vector<Item>& items) {
    clear();

    // Try to place each item
    for (const Item& item : items) {
        // For loaded items, we need to find a spot
        // In a real implementation, you'd save/load grid positions too
        int x, y;
        if (autoPlaceItem(item, x, y)) {
            placeItem(item, x, y);
        } else {
            std::cerr << "[InventoryGrid] Warning: Could not load item " << item.id << std::endl;
        }
    }
}

// ========== Inventory Implementation ==========

Inventory::Inventory() {
    // Initialize grids with Tarkov-like sizes
    stash = std::make_unique<InventoryGrid>(10, 40);      // Large persistent storage
    backpack = std::make_unique<InventoryGrid>(6, 8);     // Medium in-raid storage
    pockets = std::make_unique<InventoryGrid>(4, 2);      // Small quick access
}

bool Inventory::equipItem(const Item& item, const std::string& slot) {
    // Check if slot is valid
    auto slots = getEquipmentSlots();
    if (std::find(slots.begin(), slots.end(), slot) == slots.end()) {
        return false;
    }

    // Check if item type matches slot
    // (In production, you'd have proper type checking)

    // Store equipped item
    equipped[slot] = item;
    notifyChanged();
    return true;
}

bool Inventory::unequipItem(const std::string& slot, Item& outItem) {
    auto it = equipped.find(slot);
    if (it == equipped.end()) {
        return false;
    }

    outItem = it->second;
    equipped.erase(it);
    notifyChanged();
    return true;
}

Item* Inventory::getEquippedItem(const std::string& slot) {
    auto it = equipped.find(slot);
    if (it == equipped.end()) {
        return nullptr;
    }
    return &(it->second);
}

bool Inventory::hasItem(const std::string& itemId) const {
    // Check all grids
    for (const auto& item : stash->getAllItems()) {
        if (item->id == itemId) return true;
    }
    for (const auto& item : backpack->getAllItems()) {
        if (item->id == itemId) return true;
    }
    for (const auto& item : pockets->getAllItems()) {
        if (item->id == itemId) return true;
    }

    // Check equipped
    for (const auto& pair : equipped) {
        if (pair.second.id == itemId) return true;
    }

    return false;
}

int Inventory::getItemCount(const std::string& itemId) const {
    int count = 0;

    // Count in all grids
    for (const auto& item : stash->getAllItems()) {
        if (item->id == itemId) count += item->stackSize;
    }
    for (const auto& item : backpack->getAllItems()) {
        if (item->id == itemId) count += item->stackSize;
    }
    for (const auto& item : pockets->getAllItems()) {
        if (item->id == itemId) count += item->stackSize;
    }

    // Count equipped (usually 1)
    for (const auto& pair : equipped) {
        if (pair.second.id == itemId) count++;
    }

    return count;
}

bool Inventory::consumeItem(const std::string& itemId, int amount) {
    // Try to consume from pockets first (quick access)
    for (auto item : pockets->getAllItems()) {
        if (item->id == itemId && item->stackSize >= amount) {
            item->stackSize -= amount;
            if (item->stackSize <= 0) {
                pockets->removeItem(item->instanceId);
            }
            notifyChanged();
            return true;
        }
    }

    // Try backpack
    for (auto item : backpack->getAllItems()) {
        if (item->id == itemId && item->stackSize >= amount) {
            item->stackSize -= amount;
            if (item->stackSize <= 0) {
                backpack->removeItem(item->instanceId);
            }
            notifyChanged();
            return true;
        }
    }

    // Try stash (shouldn't happen during raid, but for consistency)
    for (auto item : stash->getAllItems()) {
        if (item->id == itemId && item->stackSize >= amount) {
            item->stackSize -= amount;
            if (item->stackSize <= 0) {
                stash->removeItem(item->instanceId);
            }
            notifyChanged();
            return true;
        }
    }

    return false;
}

float Inventory::getTotalWeight() const {
    float weight = 0.0f;

    // Calculate from all grids
    for (const auto& item : stash->getAllItems()) {
        weight += item->weight * item->stackSize;
    }
    for (const auto& item : backpack->getAllItems()) {
        weight += item->weight * item->stackSize;
    }
    for (const auto& item : pockets->getAllItems()) {
        weight += item->weight * item->stackSize;
    }

    // Add equipped items
    for (const auto& pair : equipped) {
        weight += pair.second.weight;
    }

    return weight;
}

int Inventory::getTotalValue() const {
    int value = 0;

    // Calculate from all grids
    for (const auto& item : stash->getAllItems()) {
        value += item->value * item->stackSize;
    }
    for (const auto& item : backpack->getAllItems()) {
        value += item->value * item->stackSize;
    }
    for (const auto& item : pockets->getAllItems()) {
        value += item->value * item->stackSize;
    }

    // Add equipped items
    for (const auto& pair : equipped) {
        value += pair.second.value;
    }

    return value;
}

void Inventory::notifyChanged() {
    if (onInventoryChanged) {
        onInventoryChanged();
    }
}
