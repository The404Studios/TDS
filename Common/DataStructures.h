#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <ctime>
#include <cmath>

// Core data structures shared between client and server

// ============================================================================
// ACCOUNT & AUTHENTICATION
// ============================================================================

struct Account {
    uint64_t accountId;
    std::string username;
    std::string passwordHash;  // SHA-256
    std::string email;
    uint64_t created;          // Unix timestamp
    uint64_t lastLogin;        // Unix timestamp

    Account() : accountId(0), created(0), lastLogin(0) {}
};

// ============================================================================
// ITEM SYSTEM
// ============================================================================

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

struct Item {
    uint32_t instanceId;       // Unique instance ID (0 = template)
    std::string id;            // Item type ID (e.g., "ak74")
    std::string name;          // Display name
    ItemType type;
    ItemRarity rarity;

    // Grid properties
    int width;                 // 1-4
    int height;                // 1-4

    // Stack properties
    int stackSize;             // Current stack count
    int maxStack;              // Max stack size (1 = non-stackable)

    // Value
    int value;                 // Base value in roubles
    bool foundInRaid;          // FiR status

    // Weapon properties
    int damage;
    int magazineSize;
    int currentAmmo;
    float fireRate;            // Rounds per minute
    float reloadTime;          // Seconds

    // Armor properties
    int armorClass;            // 1-6
    int durability;
    int maxDurability;

    // Medical properties
    int healAmount;
    float useTime;             // Seconds

    // Container properties
    int storageWidth;          // For backpacks/containers
    int storageHeight;

    Item() : instanceId(0), width(1), height(1), stackSize(1), maxStack(1),
             value(0), foundInRaid(false), damage(0), magazineSize(0),
             currentAmmo(0), fireRate(600.0f), reloadTime(2.5f),
             armorClass(0), durability(0), maxDurability(0),
             healAmount(0), useTime(0.0f), storageWidth(0), storageHeight(0) {}
};

// ============================================================================
// PLAYER DATA
// ============================================================================

struct PlayerStats {
    int level;
    int experience;
    int roubles;
    int raidsCompleted;
    int raidsExtracted;
    int raidsDied;
    int kills;
    int deaths;
    float survivalRate;

    PlayerStats() : level(1), experience(0), roubles(500000),
                    raidsCompleted(0), raidsExtracted(0), raidsDied(0),
                    kills(0), deaths(0), survivalRate(0.0f) {}
};

struct PlayerData {
    uint64_t accountId;
    std::string username;
    PlayerStats stats;
    std::vector<Item> stash;         // Persistent stash
    std::vector<Item> loadout;       // Current equipped gear

    PlayerData() : accountId(0) {}
};

// ============================================================================
// LOBBY SYSTEM
// ============================================================================

enum class LobbyState {
    WAITING,      // Waiting for players
    READY,        // All players ready
    IN_QUEUE,     // Searching for match
    IN_MATCH      // Currently in raid
};

struct LobbyMember {
    uint64_t accountId;
    std::string username;
    bool isReady;
    bool isOwner;

    LobbyMember() : accountId(0), isReady(false), isOwner(false) {}
};

struct Lobby {
    uint64_t lobbyId;
    uint64_t ownerId;
    std::string lobbyName;
    std::vector<LobbyMember> members;
    int maxPlayers;            // 1-5
    bool isPrivate;
    LobbyState state;
    uint64_t created;          // Unix timestamp

    Lobby() : lobbyId(0), ownerId(0), maxPlayers(5),
              isPrivate(false), state(LobbyState::WAITING), created(0) {}

    bool isFull() const {
        return (int)members.size() >= maxPlayers;
    }

    bool allReady() const {
        if (members.empty()) return false;
        for (const auto& member : members) {
            if (!member.isReady) return false;
        }
        return true;
    }

    LobbyMember* findMember(uint64_t accountId) {
        for (auto& member : members) {
            if (member.accountId == accountId) {
                return &member;
            }
        }
        return nullptr;
    }
};

// ============================================================================
// MATCH SYSTEM
// ============================================================================

enum class MatchState {
    STARTING,      // Match is being created
    ACTIVE,        // Players in raid
    ENDING,        // Match ending (timer expired or all extracted)
    FINISHED       // Match complete
};

struct MatchPlayer {
    uint64_t accountId;
    std::string username;
    float x, y, z;             // Current position
    float yaw, pitch;          // Rotation
    float health;              // Current health
    bool alive;
    bool extracted;
    std::vector<Item> lootCollected;

    MatchPlayer() : accountId(0), x(0), y(0), z(0), yaw(0), pitch(0),
                    health(440.0f), alive(true), extracted(false) {}
};

struct Match {
    uint64_t matchId;
    std::string mapName;
    std::vector<MatchPlayer> players;
    MatchState state;
    float startTime;           // Seconds since epoch
    float raidDuration;        // Seconds (default 1800 = 30 min)
    bool active;

    Match() : matchId(0), mapName("Factory"), state(MatchState::STARTING),
              startTime(0), raidDuration(1800.0f), active(false) {}

    MatchPlayer* findPlayer(uint64_t accountId) {
        for (auto& player : players) {
            if (player.accountId == accountId) {
                return &player;
            }
        }
        return nullptr;
    }

    int getAlivePlayers() const {
        int count = 0;
        for (const auto& player : players) {
            if (player.alive) count++;
        }
        return count;
    }

    bool allExtractedOrDead() const {
        for (const auto& player : players) {
            if (player.alive && !player.extracted) {
                return false;
            }
        }
        return true;
    }
};

// ============================================================================
// FRIEND SYSTEM
// ============================================================================

enum class FriendStatus {
    PENDING,       // Request sent, awaiting response
    ACCEPTED,      // Friends
    BLOCKED        // Blocked user
};

struct Friend {
    uint64_t accountId;
    std::string username;
    FriendStatus status;
    bool isOnline;
    uint64_t lobbyId;          // 0 if not in lobby
    uint64_t created;          // Unix timestamp

    Friend() : accountId(0), status(FriendStatus::PENDING),
               isOnline(false), lobbyId(0), created(0) {}
};

// ============================================================================
// MERCHANT SYSTEM
// ============================================================================

enum class MerchantType {
    FENCE = 0,         // Buys/sells everything (low prices)
    PRAPOR = 1,        // Weapons & ammo
    THERAPIST = 2,     // Medical supplies
    PEACEKEEPER = 3,   // Western gear (expensive)
    RAGMAN = 4         // Armor & clothing
};

struct MerchantOffer {
    uint32_t itemId;
    std::string itemName;
    int price;             // In roubles
    int stock;             // 0 = unlimited
    float markup;          // Price multiplier

    MerchantOffer() : itemId(0), price(0), stock(0), markup(1.0f) {}
};

struct Merchant {
    MerchantType type;
    std::string name;
    std::vector<MerchantOffer> offers;
    float buyPriceMultiplier;   // How much they pay for items
    float sellPriceMultiplier;  // How much they charge

    Merchant() : type(MerchantType::FENCE),
                 buyPriceMultiplier(0.6f),
                 sellPriceMultiplier(1.0f) {}
};

// ============================================================================
// EXTRACTION ZONES
// ============================================================================

struct ExtractionZone {
    std::string name;
    float x, y, z;
    float radius;
    float extractTime;         // Seconds to extract
    bool requiresKey;
    std::string requiredKeyId; // Item ID of key needed
    bool active;               // Can change during raid

    ExtractionZone() : x(0), y(0), z(0), radius(5.0f),
                       extractTime(8.0f), requiresKey(false),
                       active(true) {}
};

// ============================================================================
// LOOT SPAWNS
// ============================================================================

struct LootSpawn {
    uint64_t entityId;         // Unique ID for this loot instance
    Item item;
    float x, y, z;
    bool collected;

    LootSpawn() : entityId(0), x(0), y(0), z(0), collected(false) {}
};

// ============================================================================
// AI ENEMIES
// ============================================================================

enum class AIType {
    SCAV,          // Basic enemy
    RAIDER,        // Tough enemy
    BOSS,          // Boss enemy
    GUARD          // Boss guard
};

struct AIEnemy {
    uint64_t entityId;
    AIType type;
    float x, y, z;
    float yaw;
    float health;
    float maxHealth;
    bool alive;
    bool aggroed;
    uint64_t targetPlayerId;   // 0 if no target
    std::vector<Item> loot;

    AIEnemy() : entityId(0), type(AIType::SCAV),
                x(0), y(0), z(0), yaw(0),
                health(100.0f), maxHealth(100.0f),
                alive(true), aggroed(false), targetPlayerId(0) {}
};

// ============================================================================
// SESSION DATA
// ============================================================================

struct Session {
    uint64_t sessionToken;
    uint64_t accountId;
    uint64_t created;          // Unix timestamp
    uint64_t lastActivity;     // Unix timestamp
    std::string ipAddress;
    bool valid;

    Session() : sessionToken(0), accountId(0), created(0),
                lastActivity(0), valid(false) {}

    bool isExpired(uint64_t currentTime, uint64_t timeoutSeconds = 3600) const {
        return (currentTime - lastActivity) > timeoutSeconds;
    }
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

inline uint64_t getCurrentTimestamp() {
    return static_cast<uint64_t>(std::time(nullptr));
}

inline float calculateDistance3D(float x1, float y1, float z1, float x2, float y2, float z2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float dz = z2 - z1;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

inline float calculateDistance2D(float x1, float z1, float x2, float z2) {
    float dx = x2 - x1;
    float dz = z2 - z1;
    return std::sqrt(dx * dx + dz * dz);
}
