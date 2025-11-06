#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

// Item types for the extraction shooter
enum class ItemType {
    WEAPON,
    AMMO,
    ARMOR,
    HELMET,
    BACKPACK,
    MEDICAL,
    FOOD,
    MATERIAL,
    VALUABLE,
    KEY,
    CONTAINER
};

enum class ItemRarity {
    COMMON,
    UNCOMMON,
    RARE,
    EPIC,
    LEGENDARY
};

// Individual item in the game
struct Item {
    std::string id;              // Unique item ID
    std::string name;            // Display name
    ItemType type;               // Item category
    ItemRarity rarity;           // Item rarity
    int width;                   // Grid width (1-4)
    int height;                  // Grid height (1-4)
    int stackSize;               // Current stack count
    int maxStack;                // Max stack size (1 for non-stackable)
    int value;                   // Roubles value
    bool foundInRaid;            // FiR status

    // Weapon-specific
    int damage;
    int magazineSize;
    int currentAmmo;

    // Armor-specific
    int armorClass;              // 1-6
    int durability;
    int maxDurability;

    // Medical-specific
    int healAmount;
    float useTime;               // Seconds to use

    Item() : width(1), height(1), stackSize(1), maxStack(1), value(0),
             foundInRaid(false), damage(0), magazineSize(0), currentAmmo(0),
             armorClass(0), durability(0), maxDurability(0), healAmount(0), useTime(0.0f) {}
};

// Grid-based inventory slot
struct InventorySlot {
    int x, y;                    // Grid position
    std::shared_ptr<Item> item;  // Item in this slot (nullptr if empty)
    bool occupied;               // Is this slot occupied by an item

    InventorySlot() : x(0), y(0), item(nullptr), occupied(false) {}
    InventorySlot(int x, int y) : x(x), y(y), item(nullptr), occupied(false) {}
};

// Inventory container (stash, raid inventory, containers)
class Inventory {
public:
    std::string name;
    int gridWidth;               // Number of columns
    int gridHeight;              // Number of rows
    std::vector<std::vector<InventorySlot>> grid;

    Inventory(const std::string& name, int width, int height)
        : name(name), gridWidth(width), gridHeight(height) {
        // Initialize grid
        grid.resize(height);
        for (int y = 0; y < height; y++) {
            grid[y].resize(width);
            for (int x = 0; x < width; x++) {
                grid[y][x] = InventorySlot(x, y);
            }
        }
    }

    // Try to add item to inventory at specific position
    bool addItemAt(std::shared_ptr<Item> item, int x, int y) {
        if (!item) return false;

        // Check if item fits at this position
        if (x + item->width > gridWidth || y + item->height > gridHeight) {
            return false;
        }

        // Check if all required slots are empty
        for (int dy = 0; dy < item->height; dy++) {
            for (int dx = 0; dx < item->width; dx++) {
                if (grid[y + dy][x + dx].occupied) {
                    return false;
                }
            }
        }

        // Place item
        grid[y][x].item = item;
        for (int dy = 0; dy < item->height; dy++) {
            for (int dx = 0; dx < item->width; dx++) {
                grid[y + dy][x + dx].occupied = true;
            }
        }

        return true;
    }

    // Try to add item anywhere it fits
    bool addItem(std::shared_ptr<Item> item) {
        if (!item) return false;

        // Try to find empty space
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (addItemAt(item, x, y)) {
                    return true;
                }
            }
        }

        return false; // No space found
    }

    // Remove item at position
    std::shared_ptr<Item> removeItemAt(int x, int y) {
        if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
            return nullptr;
        }

        if (!grid[y][x].item) {
            return nullptr;
        }

        std::shared_ptr<Item> item = grid[y][x].item;

        // Clear all occupied slots
        for (int dy = 0; dy < item->height; dy++) {
            for (int dx = 0; dx < item->width; dx++) {
                grid[y + dy][x + dx].occupied = false;
            }
        }

        grid[y][x].item = nullptr;
        return item;
    }

    // Get item at position (returns the actual item even if clicking on occupied space)
    std::shared_ptr<Item> getItemAt(int x, int y) {
        if (x < 0 || x >= gridWidth || y < 0 || y >= gridHeight) {
            return nullptr;
        }

        // Find the root item (search backwards)
        for (int sy = y; sy >= 0; sy--) {
            for (int sx = x; sx >= 0; sx--) {
                if (grid[sy][sx].item) {
                    auto item = grid[sy][sx].item;
                    // Check if this item covers the clicked position
                    if (sx <= x && x < sx + item->width &&
                        sy <= y && y < sy + item->height) {
                        return item;
                    }
                }
            }
        }

        return nullptr;
    }

    // Get total weight
    float getTotalWeight() {
        float total = 0.0f;
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (grid[y][x].item) {
                    total += (grid[y][x].item->width * grid[y][x].item->height);
                }
            }
        }
        return total;
    }

    // Get all items
    std::vector<std::shared_ptr<Item>> getAllItems() {
        std::vector<std::shared_ptr<Item>> items;
        for (int y = 0; y < gridHeight; y++) {
            for (int x = 0; x < gridWidth; x++) {
                if (grid[y][x].item) {
                    items.push_back(grid[y][x].item);
                }
            }
        }
        return items;
    }
};

// Item database - defines all possible items in the game
class ItemDatabase {
public:
    static ItemDatabase& getInstance() {
        static ItemDatabase instance;
        return instance;
    }

    void initialize() {
        // WEAPONS
        addWeapon("ak74", "AK-74", 40, 30, 2, 4, 25000, ItemRarity::COMMON);
        addWeapon("m4a1", "M4A1", 45, 30, 2, 4, 35000, ItemRarity::UNCOMMON);
        addWeapon("svd", "SVD", 85, 10, 2, 5, 55000, ItemRarity::RARE);
        addWeapon("glock17", "Glock 17", 30, 17, 1, 2, 8000, ItemRarity::COMMON);
        addWeapon("kedr", "PP-91 Kedr", 28, 30, 1, 2, 15000, ItemRarity::COMMON);

        // AMMO
        addAmmo("545x39", "5.45x39 BP", 120, 500, ItemRarity::COMMON);
        addAmmo("556x45", "5.56x45 M855A1", 150, 600, ItemRarity::UNCOMMON);
        addAmmo("762x54", "7.62x54R SNB", 250, 1200, ItemRarity::RARE);
        addAmmo("9x18", "9x18 PM PBM", 60, 150, ItemRarity::COMMON);

        // ARMOR
        addArmor("paca", "PACA Soft Armor", 2, 50, 1, 2, 15000, ItemRarity::COMMON);
        addArmor("6b3", "6B3TM Armor", 4, 65, 2, 3, 45000, ItemRarity::UNCOMMON);
        addArmor("slick", "Slick Plate Carrier", 6, 80, 2, 2, 250000, ItemRarity::LEGENDARY);

        // HELMETS
        addHelmet("ssh68", "SSh-68", 2, 30, 2, 2, 12000, ItemRarity::COMMON);
        addHelmet("zsh", "ZSh-1-2M", 4, 40, 2, 2, 35000, ItemRarity::UNCOMMON);
        addHelmet("altyn", "Altyn Helmet", 5, 45, 2, 2, 75000, ItemRarity::RARE);

        // BACKPACKS
        addBackpack("scav", "Scav Backpack", 4, 5, 2, 3, 5000, ItemRarity::COMMON);
        addBackpack("berkut", "Berkut Backpack", 5, 6, 2, 4, 15000, ItemRarity::COMMON);
        addBackpack("trizip", "Tri-Zip Backpack", 6, 8, 3, 4, 45000, ItemRarity::UNCOMMON);

        // MEDICAL
        addMedical("ai2", "AI-2 Medkit", 30, 3.0f, 1, 1, 3000, ItemRarity::COMMON);
        addMedical("ifak", "IFAK", 50, 2.5f, 1, 1, 8000, ItemRarity::UNCOMMON);
        addMedical("grizzly", "Grizzly First Aid Kit", 175, 5.0f, 2, 2, 25000, ItemRarity::RARE);
        addMedical("surv12", "Surv12 Field Surgical Kit", 100, 10.0f, 2, 1, 45000, ItemRarity::EPIC);

        // FOOD & WATER
        addFood("tushonka", "Tushonka", 60, 1, 1, 15000, ItemRarity::COMMON);
        addFood("mre", "MRE Ration", 80, 1, 2, 25000, ItemRarity::UNCOMMON);
        addFood("water", "Aquamari Water", 100, 1, 1, 12000, ItemRarity::COMMON);

        // VALUABLES
        addValuable("rolex", "Rolex Watch", 1, 1, 65000, ItemRarity::RARE);
        addValuable("bitcoin", "Physical Bitcoin", 1, 1, 150000, ItemRarity::EPIC);
        addValuable("ledx", "LEDX Skin Transilluminator", 1, 1, 450000, ItemRarity::LEGENDARY);
        addValuable("gpu", "Graphics Card", 2, 1, 250000, ItemRarity::EPIC);

        // MATERIALS
        addMaterial("bolts", "Bolts", 1, 1, 8000, ItemRarity::COMMON);
        addMaterial("wires", "Wires", 1, 1, 12000, ItemRarity::COMMON);
        addMaterial("gunpowder", "Gunpowder", 1, 1, 15000, ItemRarity::UNCOMMON);
    }

    std::shared_ptr<Item> createItem(const std::string& id) {
        auto it = itemTemplates.find(id);
        if (it != itemTemplates.end()) {
            return std::make_shared<Item>(it->second);
        }
        return nullptr;
    }

private:
    std::map<std::string, Item> itemTemplates;

    ItemDatabase() {
        initialize();
    }

    void addWeapon(const std::string& id, const std::string& name, int damage, int magSize, int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::WEAPON;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.damage = damage;
        item.magazineSize = magSize;
        item.currentAmmo = magSize;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }

    void addAmmo(const std::string& id, const std::string& name, int stack, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::AMMO;
        item.rarity = rarity;
        item.width = 1;
        item.height = 1;
        item.value = value;
        item.maxStack = stack;
        item.stackSize = stack;
        itemTemplates[id] = item;
    }

    void addArmor(const std::string& id, const std::string& name, int armorClass, int durability, int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::ARMOR;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.armorClass = armorClass;
        item.durability = durability;
        item.maxDurability = durability;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }

    void addHelmet(const std::string& id, const std::string& name, int armorClass, int durability, int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::HELMET;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.armorClass = armorClass;
        item.durability = durability;
        item.maxDurability = durability;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }

    void addBackpack(const std::string& id, const std::string& name, int storageW, int storageH, int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::BACKPACK;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.maxStack = 1;
        // Note: backpack storage is handled separately in PlayerLoadout
        itemTemplates[id] = item;
    }

    void addMedical(const std::string& id, const std::string& name, int heal, float useTime, int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::MEDICAL;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.healAmount = heal;
        item.useTime = useTime;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }

    void addFood(const std::string& id, const std::string& name, int energy, int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::FOOD;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.healAmount = energy;
        item.useTime = 5.0f;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }

    void addValuable(const std::string& id, const std::string& name, int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::VALUABLE;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }

    void addMaterial(const std::string& id, const std::string& name, int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::MATERIAL;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }
};
