#pragma once
#include "DataStructures.h"
#include <map>
#include <memory>

// Centralized item database shared between client and server
// This ensures both have identical item definitions

class ItemDatabase {
public:
    static ItemDatabase& getInstance() {
        static ItemDatabase instance;
        return instance;
    }

    // Get item template by ID
    Item* getItemTemplate(const std::string& id) {
        auto it = itemTemplates.find(id);
        if (it != itemTemplates.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Create a new item instance from template
    Item createItem(const std::string& id, uint32_t instanceId = 0) {
        auto it = itemTemplates.find(id);
        if (it != itemTemplates.end()) {
            Item item = it->second;
            item.instanceId = instanceId;
            return item;
        }
        return Item();  // Return default item if not found
    }

    // Get all item IDs
    std::vector<std::string> getAllItemIds() const {
        std::vector<std::string> ids;
        for (const auto& pair : itemTemplates) {
            ids.push_back(pair.first);
        }
        return ids;
    }

    // Get items by type
    std::vector<Item> getItemsByType(ItemType type) const {
        std::vector<Item> items;
        for (const auto& pair : itemTemplates) {
            if (pair.second.type == type) {
                items.push_back(pair.second);
            }
        }
        return items;
    }

    // Get items by rarity
    std::vector<Item> getItemsByRarity(ItemRarity rarity) const {
        std::vector<Item> items;
        for (const auto& pair : itemTemplates) {
            if (pair.second.rarity == rarity) {
                items.push_back(pair.second);
            }
        }
        return items;
    }

    // Get item name by type (for displaying generic type names)
    static const char* getItemName(ItemType type) {
        switch (type) {
            case ItemType::WEAPON: return "Weapon";
            case ItemType::ARMOR: return "Armor";
            case ItemType::HELMET: return "Helmet";
            case ItemType::BACKPACK: return "Backpack";
            case ItemType::AMMO: return "Ammo";
            case ItemType::MEDICAL: return "Medical";
            case ItemType::FOOD: return "Food";
            case ItemType::VALUABLE: return "Valuable";
            case ItemType::MATERIAL: return "Material";
            case ItemType::KEY: return "Key";
            default: return "Unknown";
        }
    }

private:
    std::map<std::string, Item> itemTemplates;

    ItemDatabase() {
        initialize();
    }

    void initialize() {
        // WEAPONS
        addWeapon("ak74", "AK-74", 40, 30, 2, 4, 25000, ItemRarity::COMMON, 600.0f, 2.5f);
        addWeapon("m4a1", "M4A1", 45, 30, 2, 4, 35000, ItemRarity::UNCOMMON, 800.0f, 2.3f);
        addWeapon("svd", "SVD", 85, 10, 2, 5, 55000, ItemRarity::RARE, 300.0f, 3.5f);
        addWeapon("glock17", "Glock 17", 30, 17, 1, 2, 8000, ItemRarity::COMMON, 500.0f, 2.0f);
        addWeapon("kedr", "PP-91 Kedr", 28, 30, 1, 2, 15000, ItemRarity::COMMON, 900.0f, 1.8f);
        addWeapon("mp5", "MP5", 35, 30, 2, 3, 28000, ItemRarity::COMMON, 800.0f, 2.2f);
        addWeapon("sks", "SKS", 55, 10, 2, 4, 32000, ItemRarity::UNCOMMON, 400.0f, 2.8f);
        addWeapon("sa58", "SA-58", 62, 20, 2, 5, 75000, ItemRarity::RARE, 650.0f, 3.0f);

        // AMMO
        addAmmo("545x39", "5.45x39 BP", 120, 500, ItemRarity::COMMON);
        addAmmo("556x45", "5.56x45 M855A1", 120, 600, ItemRarity::UNCOMMON);
        addAmmo("762x54", "7.62x54R SNB", 60, 1200, ItemRarity::RARE);
        addAmmo("9x18", "9x18 PM PBM", 120, 150, ItemRarity::COMMON);
        addAmmo("9x19", "9x19 PST gzh", 120, 250, ItemRarity::COMMON);
        addAmmo("762x39", "7.62x39 PS", 120, 400, ItemRarity::COMMON);
        addAmmo("762x51", "7.62x51 M80", 80, 800, ItemRarity::UNCOMMON);

        // ARMOR
        addArmor("paca", "PACA Soft Armor", 2, 50, 1, 2, 15000, ItemRarity::COMMON);
        addArmor("6b3", "6B3TM Armor", 4, 65, 2, 3, 45000, ItemRarity::UNCOMMON);
        addArmor("6b43", "6B43 Zabralo", 5, 85, 2, 3, 125000, ItemRarity::RARE);
        addArmor("slick", "Slick Plate Carrier", 6, 80, 2, 2, 250000, ItemRarity::LEGENDARY);
        addArmor("trooper", "Trooper Armor", 4, 75, 2, 3, 65000, ItemRarity::UNCOMMON);

        // HELMETS
        addHelmet("ssh68", "SSh-68", 2, 30, 2, 2, 12000, ItemRarity::COMMON);
        addHelmet("zsh", "ZSh-1-2M", 4, 40, 2, 2, 35000, ItemRarity::UNCOMMON);
        addHelmet("altyn", "Altyn Helmet", 5, 45, 2, 2, 75000, ItemRarity::RARE);
        addHelmet("exfil", "EXFIL Helmet", 4, 35, 2, 2, 55000, ItemRarity::UNCOMMON);
        addHelmet("fast_mt", "FAST MT", 4, 30, 2, 2, 45000, ItemRarity::UNCOMMON);

        // BACKPACKS
        addBackpack("scav", "Scav Backpack", 4, 5, 2, 3, 5000, ItemRarity::COMMON);
        addBackpack("berkut", "Berkut Backpack", 5, 6, 2, 4, 15000, ItemRarity::COMMON);
        addBackpack("trizip", "Tri-Zip Backpack", 6, 8, 3, 4, 45000, ItemRarity::UNCOMMON);
        addBackpack("attack2", "Attack 2 Backpack", 5, 7, 2, 4, 35000, ItemRarity::UNCOMMON);
        addBackpack("pilgrim", "Pilgrim Backpack", 6, 7, 3, 5, 55000, ItemRarity::RARE);

        // MEDICAL
        addMedical("ai2", "AI-2 Medkit", 30, 3.0f, 1, 1, 3000, ItemRarity::COMMON);
        addMedical("ifak", "IFAK", 50, 2.5f, 1, 1, 8000, ItemRarity::UNCOMMON);
        addMedical("grizzly", "Grizzly First Aid Kit", 175, 5.0f, 2, 2, 25000, ItemRarity::RARE);
        addMedical("surv12", "Surv12 Field Surgical Kit", 100, 10.0f, 2, 1, 45000, ItemRarity::EPIC);
        addMedical("salewa", "Salewa First Aid Kit", 120, 4.0f, 1, 2, 12000, ItemRarity::COMMON);
        addMedical("morphine", "Morphine Injector", 0, 1.0f, 1, 1, 8000, ItemRarity::UNCOMMON);  // Pain relief

        // FOOD & WATER
        addFood("tushonka", "Tushonka", 60, 1, 1, 15000, ItemRarity::COMMON);
        addFood("mre", "MRE Ration", 80, 1, 2, 25000, ItemRarity::UNCOMMON);
        addFood("water", "Aquamari Water", 100, 1, 1, 12000, ItemRarity::COMMON);
        addFood("condensed_milk", "Condensed Milk", 70, 1, 1, 18000, ItemRarity::COMMON);
        addFood("energy_drink", "Energy Drink", 50, 1, 1, 22000, ItemRarity::UNCOMMON);

        // VALUABLES (HIGH VALUE LOOT)
        addValuable("rolex", "Rolex Watch", 1, 1, 65000, ItemRarity::RARE);
        addValuable("bitcoin", "Physical Bitcoin", 1, 1, 150000, ItemRarity::EPIC);
        addValuable("ledx", "LEDX Skin Transilluminator", 1, 1, 450000, ItemRarity::LEGENDARY);
        addValuable("gpu", "Graphics Card", 2, 1, 250000, ItemRarity::EPIC);
        addValuable("tetriz", "Tetriz Game", 1, 1, 35000, ItemRarity::UNCOMMON);
        addValuable("lion", "Lion Statue", 2, 2, 180000, ItemRarity::EPIC);
        addValuable("skull", "Skull", 1, 1, 75000, ItemRarity::RARE);
        addValuable("firesteel", "Firesteel", 1, 1, 28000, ItemRarity::UNCOMMON);
        addValuable("vase", "Antique Vase", 2, 2, 95000, ItemRarity::RARE);

        // MATERIALS (CRAFTING/TRADING)
        addMaterial("bolts", "Bolts", 1, 1, 8000, ItemRarity::COMMON);
        addMaterial("wires", "Wires", 1, 1, 12000, ItemRarity::COMMON);
        addMaterial("gunpowder", "Gunpowder", 1, 1, 15000, ItemRarity::UNCOMMON);
        addMaterial("screw_nuts", "Screw Nuts", 1, 1, 10000, ItemRarity::COMMON);
        addMaterial("capacitors", "Capacitors", 1, 1, 18000, ItemRarity::UNCOMMON);
        addMaterial("cpu", "CPU", 1, 1, 35000, ItemRarity::RARE);
        addMaterial("circuit", "Circuit Board", 1, 1, 25000, ItemRarity::UNCOMMON);

        // KEYS (For locked areas)
        addKey("factory_key", "Factory Key", 1, 1, 85000, ItemRarity::RARE);
        addKey("marked_key", "Marked Room Key", 1, 1, 125000, ItemRarity::EPIC);
        addKey("cottage_key", "Cottage Key", 1, 1, 45000, ItemRarity::UNCOMMON);
    }

    void addWeapon(const std::string& id, const std::string& name, int damage,
                   int magSize, int w, int h, int value, ItemRarity rarity,
                   float fireRate, float reloadTime) {
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
        item.fireRate = fireRate;
        item.reloadTime = reloadTime;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }

    void addAmmo(const std::string& id, const std::string& name,
                 int maxStack, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::AMMO;
        item.rarity = rarity;
        item.width = 1;
        item.height = 1;
        item.value = value;
        item.maxStack = maxStack;
        item.stackSize = maxStack;
        itemTemplates[id] = item;
    }

    void addArmor(const std::string& id, const std::string& name,
                  int armorClass, int durability, int w, int h,
                  int value, ItemRarity rarity) {
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

    void addHelmet(const std::string& id, const std::string& name,
                   int armorClass, int durability, int w, int h,
                   int value, ItemRarity rarity) {
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

    void addBackpack(const std::string& id, const std::string& name,
                     int storageW, int storageH, int w, int h,
                     int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::BACKPACK;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.storageWidth = storageW;
        item.storageHeight = storageH;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }

    void addMedical(const std::string& id, const std::string& name,
                    int heal, float useTime, int w, int h,
                    int value, ItemRarity rarity) {
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

    void addFood(const std::string& id, const std::string& name,
                 int energy, int w, int h, int value, ItemRarity rarity) {
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

    void addValuable(const std::string& id, const std::string& name,
                     int w, int h, int value, ItemRarity rarity) {
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

    void addMaterial(const std::string& id, const std::string& name,
                     int w, int h, int value, ItemRarity rarity) {
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

    void addKey(const std::string& id, const std::string& name,
                int w, int h, int value, ItemRarity rarity) {
        Item item;
        item.id = id;
        item.name = name;
        item.type = ItemType::KEY;
        item.rarity = rarity;
        item.width = w;
        item.height = h;
        item.value = value;
        item.maxStack = 1;
        itemTemplates[id] = item;
    }
};
