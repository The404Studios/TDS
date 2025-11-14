#include "Items.h"

namespace TDS {

std::vector<ItemDefinition> ItemDatabase::items;
std::vector<WeaponData> ItemDatabase::weapons;
std::vector<ArmorData> ItemDatabase::armors;
bool ItemDatabase::initialized = false;

void ItemDatabase::initialize() {
    if (initialized) return;

    // ========================================================================
    // Weapons
    // ========================================================================

    items.push_back({Items::AK74, "AK-74", "5.45x39mm assault rifle",
        ItemCategory::WEAPON, ItemRarity::COMMON, 25000, 3600, 4, 2, false, 1,
        "models/weapons/ak74.obj", "textures/icons/ak74.png"});

    items.push_back({Items::M4A1, "M4A1", "5.56x45mm assault rifle",
        ItemCategory::WEAPON, ItemRarity::RARE, 45000, 3200, 4, 2, false, 1,
        "models/weapons/m4a1.obj", "textures/icons/m4a1.png"});

    items.push_back({Items::SVD, "SVD", "7.62x54R sniper rifle",
        ItemCategory::WEAPON, ItemRarity::EPIC, 65000, 4300, 5, 2, false, 1,
        "models/weapons/svd.obj", "textures/icons/svd.png"});

    items.push_back({Items::GLOCK17, "Glock 17", "9x19mm pistol",
        ItemCategory::WEAPON, ItemRarity::COMMON, 8000, 910, 2, 2, false, 1,
        "models/weapons/glock17.obj", "textures/icons/glock17.png"});

    items.push_back({Items::MP5, "MP5", "9x19mm submachine gun",
        ItemCategory::WEAPON, ItemRarity::UNCOMMON, 22000, 2900, 4, 2, false, 1,
        "models/weapons/mp5.obj", "textures/icons/mp5.png"});

    items.push_back({Items::MOSIN, "Mosin", "7.62x54R bolt-action rifle",
        ItemCategory::WEAPON, ItemRarity::COMMON, 18000, 4000, 5, 1, false, 1,
        "models/weapons/mosin.obj", "textures/icons/mosin.png"});

    // Weapon data
    weapons.push_back({Items::AK74, WeaponType::ASSAULT_RIFLE, 42, 650.0f, 30, 2.3f, 400.0f, 0.75f, 1.8f, Items::AMMO_545x39, "sounds/weapons/ak74_fire.ogg", "sounds/weapons/reload.ogg"});
    weapons.push_back({Items::M4A1, WeaponType::ASSAULT_RIFLE, 45, 800.0f, 30, 2.1f, 450.0f, 0.82f, 1.5f, Items::AMMO_556x45, "sounds/weapons/m4_fire.ogg", "sounds/weapons/reload.ogg"});
    weapons.push_back({Items::SVD, WeaponType::SNIPER_RIFLE, 85, 300.0f, 10, 3.5f, 800.0f, 0.92f, 2.5f, Items::AMMO_762x54R, "sounds/weapons/svd_fire.ogg", "sounds/weapons/reload.ogg"});
    weapons.push_back({Items::GLOCK17, WeaponType::PISTOL, 35, 450.0f, 17, 1.5f, 50.0f, 0.68f, 0.8f, Items::AMMO_9x19, "sounds/weapons/pistol_fire.ogg", "sounds/weapons/reload.ogg"});
    weapons.push_back({Items::MP5, WeaponType::SMG, 38, 800.0f, 30, 2.0f, 150.0f, 0.78f, 1.2f, Items::AMMO_9x19, "sounds/weapons/mp5_fire.ogg", "sounds/weapons/reload.ogg"});
    weapons.push_back({Items::MOSIN, WeaponType::SNIPER_RIFLE, 95, 60.0f, 5, 4.0f, 1000.0f, 0.95f, 3.2f, Items::AMMO_762x54R, "sounds/weapons/mosin_fire.ogg", "sounds/weapons/reload.ogg"});

    // ========================================================================
    // Ammo
    // ========================================================================

    items.push_back({Items::AMMO_545x39, "5.45x39mm", "AK-74 ammunition",
        ItemCategory::AMMO, ItemRarity::COMMON, 350, 10, 1, 1, true, 60,
        "models/items/ammo_box.obj", "textures/icons/ammo_545.png"});

    items.push_back({Items::AMMO_556x45, "5.56x45mm", "M4A1 ammunition",
        ItemCategory::AMMO, ItemRarity::UNCOMMON, 450, 12, 1, 1, true, 60,
        "models/items/ammo_box.obj", "textures/icons/ammo_556.png"});

    items.push_back({Items::AMMO_9x19, "9x19mm", "Pistol/SMG ammunition",
        ItemCategory::AMMO, ItemRarity::COMMON, 200, 8, 1, 1, true, 50,
        "models/items/ammo_box.obj", "textures/icons/ammo_9mm.png"});

    // ========================================================================
    // Armor
    // ========================================================================

    items.push_back({Items::PACA_ARMOR, "PACA Soft Armor", "Class 2 soft armor",
        ItemCategory::ARMOR, ItemRarity::COMMON, 12000, 4200, 3, 3, false, 1,
        "models/armor/paca.obj", "textures/icons/paca.png"});

    items.push_back({Items::CLASS4_ARMOR, "6B3TM Armor", "Class 4 armor rig",
        ItemCategory::ARMOR, ItemRarity::UNCOMMON, 45000, 8500, 3, 3, false, 1,
        "models/armor/class4.obj", "textures/icons/class4.png"});

    items.push_back({Items::CLASS6_ARMOR, "FORT Armor", "Class 6 heavy armor",
        ItemCategory::ARMOR, ItemRarity::LEGENDARY, 180000, 18000, 3, 4, false, 1,
        "models/armor/fort.obj", "textures/icons/fort.png"});

    armors.push_back({Items::PACA_ARMOR, 2, 50, 50, 0.05f});
    armors.push_back({Items::CLASS4_ARMOR, 4, 65, 65, 0.12f});
    armors.push_back({Items::CLASS6_ARMOR, 6, 85, 85, 0.25f});

    // ========================================================================
    // Medical
    // ========================================================================

    items.push_back({Items::IFAK, "IFAK", "Individual First Aid Kit",
        ItemCategory::MEDICAL, ItemRarity::UNCOMMON, 8500, 200, 1, 1, false, 1,
        "models/items/ifak.obj", "textures/icons/ifak.png"});

    items.push_back({Items::SALEWA, "Salewa", "First aid kit",
        ItemCategory::MEDICAL, ItemRarity::COMMON, 15000, 350, 1, 2, false, 1,
        "models/items/salewa.obj", "textures/icons/salewa.png"});

    items.push_back({Items::GRIZZLY, "Grizzly", "Large medical kit",
        ItemCategory::MEDICAL, ItemRarity::RARE, 28000, 1100, 2, 2, false, 1,
        "models/items/grizzly.obj", "textures/icons/grizzly.png"});

    // ========================================================================
    // Valuables
    // ========================================================================

    items.push_back({Items::ROLEX, "Rolex", "Expensive watch",
        ItemCategory::VALUABLE, ItemRarity::RARE, 85000, 50, 1, 1, false, 1,
        "models/items/rolex.obj", "textures/icons/rolex.png"});

    items.push_back({Items::BITCOIN, "Bitcoin", "Physical bitcoin",
        ItemCategory::VALUABLE, ItemRarity::EPIC, 250000, 20, 1, 1, true, 50,
        "models/items/bitcoin.obj", "textures/icons/bitcoin.png"});

    items.push_back({Items::LEDX, "LEDX", "Medical device",
        ItemCategory::VALUABLE, ItemRarity::LEGENDARY, 680000, 650, 1, 1, false, 1,
        "models/items/ledx.obj", "textures/icons/ledx.png"});

    items.push_back({Items::GPU, "Graphics Card", "High-end GPU",
        ItemCategory::VALUABLE, ItemRarity::EPIC, 320000, 450, 2, 1, false, 1,
        "models/items/gpu.obj", "textures/icons/gpu.png"});

    // ========================================================================
    // Food/Water
    // ========================================================================

    items.push_back({Items::WATER_BOTTLE, "Water", "Aquamari water bottle",
        ItemCategory::FOOD, ItemRarity::COMMON, 5000, 400, 1, 2, false, 1,
        "models/items/water.obj", "textures/icons/water.png"});

    items.push_back({Items::TUSHONKA, "Tushonka", "Canned meat",
        ItemCategory::FOOD, ItemRarity::COMMON, 8000, 325, 1, 1, false, 1,
        "models/items/tushonka.obj", "textures/icons/tushonka.png"});

    initialized = true;
}

const ItemDefinition* ItemDatabase::getItem(uint16_t id) {
    for (const auto& item : items) {
        if (item.id == id) return &item;
    }
    return nullptr;
}

const WeaponData* ItemDatabase::getWeapon(uint16_t id) {
    for (const auto& weapon : weapons) {
        if (weapon.itemId == id) return &weapon;
    }
    return nullptr;
}

const ArmorData* ItemDatabase::getArmor(uint16_t id) {
    for (const auto& armor : armors) {
        if (armor.itemId == id) return &armor;
    }
    return nullptr;
}

const std::vector<ItemDefinition>& ItemDatabase::getAllItems() {
    return items;
}

} // namespace TDS
