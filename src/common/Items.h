#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace TDS {

// ============================================================================
// Item Categories
// ============================================================================

enum class ItemCategory : uint8_t {
    WEAPON,
    AMMO,
    ARMOR,
    HELMET,
    BACKPACK,
    MEDICAL,
    FOOD,
    VALUABLE,
    MATERIAL,
    KEY,
    ATTACHMENT,
};

enum class ItemRarity : uint8_t {
    COMMON,
    UNCOMMON,
    RARE,
    EPIC,
    LEGENDARY,
};

enum class WeaponType : uint8_t {
    ASSAULT_RIFLE,
    SMG,
    SNIPER_RIFLE,
    PISTOL,
    SHOTGUN,
    DMR,
};

// ============================================================================
// Item Definition
// ============================================================================

struct ItemDefinition {
    uint16_t id;
    std::string name;
    std::string description;
    ItemCategory category;
    ItemRarity rarity;
    uint32_t baseValue;      // Base price in roubles
    uint16_t weight;         // Weight in grams
    uint8_t gridWidth;       // Inventory grid width
    uint8_t gridHeight;      // Inventory grid height
    bool stackable;
    uint16_t maxStack;
    std::string modelPath;   // Path to 3D model
    std::string iconPath;    // Path to inventory icon
};

// Weapon-specific data
struct WeaponData {
    uint16_t itemId;
    WeaponType type;
    uint16_t damage;
    float fireRate;          // Rounds per minute
    uint8_t magazineSize;
    float reloadTime;        // Seconds
    float range;             // Meters
    float accuracy;          // 0.0 - 1.0
    float recoil;            // Recoil strength
    uint16_t ammoType;       // Item ID of compatible ammo
    std::string fireSound;
    std::string reloadSound;
};

// Armor data
struct ArmorData {
    uint16_t itemId;
    uint8_t armorClass;      // 1-6 (Tarkov style)
    uint16_t durability;
    uint16_t maxDurability;
    float movementPenalty;   // 0.0 - 1.0
};

// ============================================================================
// Item Database
// ============================================================================

class ItemDatabase {
public:
    static void initialize();
    static const ItemDefinition* getItem(uint16_t id);
    static const WeaponData* getWeapon(uint16_t id);
    static const ArmorData* getArmor(uint16_t id);
    static const std::vector<ItemDefinition>& getAllItems();

private:
    static std::vector<ItemDefinition> items;
    static std::vector<WeaponData> weapons;
    static std::vector<ArmorData> armors;
    static bool initialized;
};

// ============================================================================
// Predefined Item IDs
// ============================================================================

namespace Items {
    // Weapons
    constexpr uint16_t AK74 = 1;
    constexpr uint16_t M4A1 = 2;
    constexpr uint16_t SVD = 3;
    constexpr uint16_t GLOCK17 = 4;
    constexpr uint16_t MP5 = 5;
    constexpr uint16_t MOSIN = 6;
    constexpr uint16_t AK74M = 7;
    constexpr uint16_t KEDR = 8;

    // Ammo
    constexpr uint16_t AMMO_545x39 = 100;
    constexpr uint16_t AMMO_556x45 = 101;
    constexpr uint16_t AMMO_762x54R = 102;
    constexpr uint16_t AMMO_9x19 = 103;
    constexpr uint16_t AMMO_9x18 = 104;
    constexpr uint16_t AMMO_762x39 = 105;
    constexpr uint16_t AMMO_12GAUGE = 106;

    // Armor
    constexpr uint16_t PACA_ARMOR = 200;
    constexpr uint16_t CLASS3_ARMOR = 201;
    constexpr uint16_t CLASS4_ARMOR = 202;
    constexpr uint16_t CLASS5_ARMOR = 203;
    constexpr uint16_t CLASS6_ARMOR = 204;

    // Helmets
    constexpr uint16_t SSH68_HELMET = 220;
    constexpr uint16_t KOLPAK_HELMET = 221;
    constexpr uint16_t ULACH_HELMET = 222;
    constexpr uint16_t EXFIL_HELMET = 223;
    constexpr uint16_t ALTYN_HELMET = 224;

    // Backpacks
    constexpr uint16_t SCAV_BP = 240;
    constexpr uint16_t BERKUT = 241;
    constexpr uint16_t TRIZIP = 242;
    constexpr uint16_t PILGRIM = 243;
    constexpr uint16_t ATTACK2 = 244;

    // Medical
    constexpr uint16_t IFAK = 300;
    constexpr uint16_t AI2_MEDKIT = 301;
    constexpr uint16_t SALEWA = 302;
    constexpr uint16_t GRIZZLY = 303;
    constexpr uint16_t MORPHINE = 304;
    constexpr uint16_t PAINKILLERS = 305;

    // Food/Water
    constexpr uint16_t WATER_BOTTLE = 320;
    constexpr uint16_t TUSHONKA = 321;
    constexpr uint16_t HERRING = 322;
    constexpr uint16_t CONDENSED_MILK = 323;
    constexpr uint16_t ENERGY_DRINK = 324;

    // Valuables
    constexpr uint16_t ROLEX = 400;
    constexpr uint16_t BITCOIN = 401;
    constexpr uint16_t LEDX = 402;
    constexpr uint16_t GPU = 403;
    constexpr uint16_t TETRIZ = 404;
    constexpr uint16_t LION = 405;
    constexpr uint16_t CAT_STATUE = 406;
    constexpr uint16_t SKULL = 407;
    constexpr uint16_t VASE = 408;

    // Materials
    constexpr uint16_t BOLTS = 500;
    constexpr uint16_t NUTS = 501;
    constexpr uint16_t SCREW = 502;
    constexpr uint16_t WIRE = 503;
    constexpr uint16_t TAPE = 504;
    constexpr uint16_t HOSE = 505;
    constexpr uint16_t BATTERY = 506;

    // Keys
    constexpr uint16_t DORM_KEY = 600;
    constexpr uint16_t FACTORY_KEY = 601;
    constexpr uint16_t MARKED_KEY = 602;
}

} // namespace TDS
