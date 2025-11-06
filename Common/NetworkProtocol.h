#pragma once
#include "DataStructures.h"
#include <cstdint>
#include <string>
#include <vector>

// Network Protocol for Extraction Shooter
// All packets use this header + payload structure

enum class PacketType : uint16_t {
    // Authentication (0-99)
    LOGIN_REQUEST = 0,
    LOGIN_RESPONSE = 1,
    REGISTER_REQUEST = 2,
    REGISTER_RESPONSE = 3,
    LOGOUT = 4,

    // Lobby System (100-199)
    LOBBY_CREATE = 100,
    LOBBY_CREATE_RESPONSE = 101,
    LOBBY_JOIN = 102,
    LOBBY_JOIN_RESPONSE = 103,
    LOBBY_LEAVE = 104,
    LOBBY_UPDATE = 105,
    LOBBY_KICK = 106,
    LOBBY_READY = 107,
    LOBBY_START_QUEUE = 108,
    LOBBY_STOP_QUEUE = 109,

    // Friend System (200-299)
    FRIEND_REQUEST = 200,
    FRIEND_ACCEPT = 201,
    FRIEND_DECLINE = 202,
    FRIEND_REMOVE = 203,
    FRIEND_LIST_REQUEST = 204,
    FRIEND_LIST_RESPONSE = 205,
    FRIEND_STATUS_UPDATE = 206,
    FRIEND_INVITE_LOBBY = 207,

    // Match System (300-399)
    MATCH_START = 300,
    MATCH_END = 301,
    MATCH_FOUND = 302,
    SPAWN_INFO = 303,
    PLAYER_SPAWN = 304,
    EXTRACTION_COMPLETE = 305,

    // Gameplay (400-499)
    PLAYER_MOVE = 400,
    PLAYER_SHOOT = 401,
    PLAYER_DAMAGE = 402,
    PLAYER_DEATH = 403,
    PLAYER_LOOT = 404,
    PLAYER_RELOAD = 405,
    PLAYER_USE_ITEM = 406,

    // Merchant/Economy (500-599)
    MERCHANT_LIST_REQUEST = 500,
    MERCHANT_LIST_RESPONSE = 501,
    MERCHANT_BUY = 502,
    MERCHANT_SELL = 503,
    MERCHANT_TRANSACTION_RESPONSE = 504,

    // Player Data (600-699)
    PLAYER_DATA_REQUEST = 600,
    PLAYER_DATA_RESPONSE = 601,
    PLAYER_DATA_UPDATE = 602,
    STASH_UPDATE = 603,
    LOADOUT_UPDATE = 604,

    // Heartbeat & Connection (700-799)
    HEARTBEAT = 700,
    DISCONNECT = 701,
    SERVER_SHUTDOWN = 702,

    // Error Messages (800-899)
    ERROR_RESPONSE = 800,
    INVALID_PACKET = 801,
    SESSION_EXPIRED = 802,
    PERMISSION_DENIED = 803
};

// Packet header (sent with every packet)
#pragma pack(push, 1)
struct PacketHeader {
    uint16_t type;          // PacketType
    uint32_t payloadSize;   // Size of payload in bytes
    uint64_t sessionToken;  // Session authentication token
    uint32_t sequence;      // Packet sequence number (for ordering)
};
#pragma pack(pop)

// Maximum packet size (16KB)
constexpr size_t MAX_PACKET_SIZE = 16384;

// ============================================================================
// AUTHENTICATION PACKETS
// ============================================================================

struct LoginRequest {
    char username[32];
    char passwordHash[64];  // SHA-256 hash
};

struct LoginResponse {
    bool success;
    uint64_t accountId;
    uint64_t sessionToken;
    char errorMessage[256];
};

struct RegisterRequest {
    char username[32];
    char passwordHash[64];
    char email[256];
};

struct RegisterResponse {
    bool success;
    uint64_t accountId;
    char errorMessage[256];
};

// ============================================================================
// LOBBY PACKETS
// ============================================================================

struct LobbyCreateRequest {
    char lobbyName[64];
    uint8_t maxPlayers;     // 1-5
    bool isPrivate;
};

struct LobbyCreateResponse {
    bool success;
    uint64_t lobbyId;
    char errorMessage[256];
};

struct LobbyJoinRequest {
    uint64_t lobbyId;
    char password[64];      // For private lobbies
};

struct LobbyJoinResponse {
    bool success;
    uint64_t lobbyId;
    char errorMessage[256];
};

struct LobbyMemberInfo {
    uint64_t accountId;
    char username[32];
    bool isReady;
    bool isOwner;
};

struct LobbyUpdate {
    uint64_t lobbyId;
    uint8_t memberCount;
    LobbyMemberInfo members[5];  // Max 5 players
    bool inQueue;
};

struct LobbyKick {
    uint64_t targetAccountId;
};

struct LobbyReady {
    bool ready;
};

// ============================================================================
// FRIEND PACKETS
// ============================================================================

struct FriendRequest {
    char targetUsername[32];
};

struct FriendAccept {
    uint64_t friendAccountId;
};

struct FriendRemove {
    uint64_t friendAccountId;
};

struct FriendInfo {
    uint64_t accountId;
    char username[32];
    bool isOnline;
    uint64_t lobbyId;       // 0 if not in lobby
};

struct FriendListResponse {
    uint8_t friendCount;
    FriendInfo friends[100];  // Max 100 friends
};

struct FriendStatusUpdate {
    uint64_t accountId;
    bool isOnline;
    uint64_t lobbyId;
};

struct FriendInviteLobby {
    uint64_t friendAccountId;
    uint64_t lobbyId;
};

// ============================================================================
// MATCH PACKETS
// ============================================================================

struct MatchFound {
    uint64_t matchId;
    char mapName[64];
};

struct SpawnInfo {
    float spawnX;
    float spawnY;
    float spawnZ;
    float spawnYaw;
    uint8_t playerCount;
    uint64_t playerIds[5];  // All players in this match
};

struct PlayerSpawn {
    uint64_t accountId;
    float x, y, z;
    float yaw;
};

struct ExtractionComplete {
    bool extracted;         // true = extracted, false = died
    uint32_t roubles;       // Money gained/lost
    uint16_t itemCount;
    // Items follow in separate packets
};

// ============================================================================
// GAMEPLAY PACKETS
// ============================================================================

struct PlayerMove {
    float x, y, z;
    float yaw, pitch;
    uint8_t movementFlags;  // Bitfield: walking, sprinting, crouching
};

struct PlayerShoot {
    float originX, originY, originZ;
    float dirX, dirY, dirZ;
    uint32_t weaponId;
};

struct PlayerDamage {
    uint64_t targetAccountId;
    float damage;
    uint32_t weaponId;
};

struct PlayerDeath {
    uint64_t victimAccountId;
    uint64_t killerAccountId;
};

struct PlayerLoot {
    uint64_t lootEntityId;  // Item on ground or corpse
    uint32_t itemId;
};

struct PlayerReload {
    uint32_t weaponId;
};

struct PlayerUseItem {
    uint32_t itemId;
    uint8_t slotIndex;      // Inventory slot
};

// ============================================================================
// MERCHANT PACKETS
// ============================================================================

struct MerchantItem {
    uint32_t itemId;
    char itemName[64];
    uint32_t price;
    uint16_t stock;         // 0 = unlimited
};

struct MerchantListResponse {
    uint8_t merchantId;     // 0=Fence, 1=Prapor, etc.
    uint16_t itemCount;
    MerchantItem items[200]; // Max 200 items per merchant
};

struct MerchantBuy {
    uint8_t merchantId;
    uint32_t itemId;
    uint16_t quantity;
};

struct MerchantSell {
    uint8_t merchantId;
    uint32_t itemId;
    uint16_t quantity;
};

struct MerchantTransactionResponse {
    bool success;
    uint32_t newBalance;    // Updated roubles
    char errorMessage[256];
};

// ============================================================================
// PLAYER DATA PACKETS
// ============================================================================

// PlayerStats is defined in DataStructures.h

struct ItemData {
    uint32_t itemId;
    char itemName[64];
    uint16_t stackSize;
    bool foundInRaid;
    uint16_t currentAmmo;
    uint16_t durability;
};

struct PlayerDataResponse {
    uint64_t accountId;
    char username[32];
    PlayerStats stats;
    uint16_t stashItemCount;
    // Items follow in separate packets
};

struct StashUpdate {
    uint16_t itemCount;
    // Items follow
};

struct LoadoutUpdate {
    uint32_t primaryWeaponId;
    uint32_t secondaryWeaponId;
    uint32_t armorId;
    uint32_t helmetId;
    uint32_t backpackId;
};

// ============================================================================
// ERROR PACKETS
// ============================================================================

struct ErrorResponse {
    uint16_t errorCode;
    char errorMessage[256];
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

inline const char* packetTypeToString(PacketType type) {
    switch (type) {
        case PacketType::LOGIN_REQUEST: return "LOGIN_REQUEST";
        case PacketType::LOGIN_RESPONSE: return "LOGIN_RESPONSE";
        case PacketType::REGISTER_REQUEST: return "REGISTER_REQUEST";
        case PacketType::REGISTER_RESPONSE: return "REGISTER_RESPONSE";
        case PacketType::LOBBY_CREATE: return "LOBBY_CREATE";
        case PacketType::LOBBY_JOIN: return "LOBBY_JOIN";
        case PacketType::FRIEND_REQUEST: return "FRIEND_REQUEST";
        case PacketType::MATCH_START: return "MATCH_START";
        case PacketType::PLAYER_MOVE: return "PLAYER_MOVE";
        case PacketType::MERCHANT_BUY: return "MERCHANT_BUY";
        case PacketType::MERCHANT_SELL: return "MERCHANT_SELL";
        case PacketType::ERROR_RESPONSE: return "ERROR_RESPONSE";
        default: return "UNKNOWN";
    }
}
