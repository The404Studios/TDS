#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

namespace TDS {

// ============================================================================
// Packet Types
// ============================================================================

enum class PacketType : uint8_t {
    // Authentication (0-19)
    AUTH_LOGIN_REQUEST = 0,
    AUTH_LOGIN_RESPONSE = 1,
    AUTH_REGISTER_REQUEST = 2,
    AUTH_REGISTER_RESPONSE = 3,
    AUTH_LOGOUT = 4,
    AUTH_DISCONNECT = 5,

    // Lobby (20-39)
    LOBBY_CREATE = 20,
    LOBBY_JOIN = 21,
    LOBBY_LEAVE = 22,
    LOBBY_UPDATE = 23,
    LOBBY_READY = 24,
    LOBBY_START_QUEUE = 25,
    LOBBY_MATCH_FOUND = 26,
    LOBBY_INVITE = 27,

    // Friend System (40-59)
    FRIEND_REQUEST = 40,
    FRIEND_ACCEPT = 41,
    FRIEND_DECLINE = 42,
    FRIEND_REMOVE = 43,
    FRIEND_LIST = 44,
    FRIEND_STATUS = 45,

    // Match/Gameplay (60-99)
    MATCH_SPAWN = 60,
    MATCH_PLAYER_JOIN = 61,
    MATCH_PLAYER_LEAVE = 62,

    // Movement (unreliable sequenced)
    PLAYER_MOVE = 70,
    PLAYER_ROTATE = 71,
    PLAYER_JUMP = 72,
    PLAYER_CROUCH = 73,

    // Combat (unreliable sequenced for shots, reliable for damage)
    WEAPON_SHOOT = 80,
    WEAPON_RELOAD = 81,
    WEAPON_SWITCH = 82,
    DAMAGE_DEALT = 83,
    PLAYER_DEATH = 84,
    PLAYER_RESPAWN = 85,

    // Items/Loot (reliable ordered)
    LOOT_SPAWN = 90,
    LOOT_COLLECT = 91,
    LOOT_DROP = 92,
    INVENTORY_UPDATE = 93,

    // Extraction (reliable ordered)
    EXTRACTION_REQUEST = 95,
    EXTRACTION_RESPONSE = 96,

    // Merchant (100-119) (reliable ordered)
    MERCHANT_LIST = 100,
    MERCHANT_BUY = 101,
    MERCHANT_SELL = 102,
    MERCHANT_RESPONSE = 103,

    // Player Data (120-139) (reliable ordered)
    PLAYER_STATS = 120,
    PLAYER_STASH = 121,
    PLAYER_LOADOUT = 122,

    // Error/Info (140-159)
    ERROR_MESSAGE = 140,
    PING = 150,
    PONG = 151,
};

// ============================================================================
// ENet Channel Configuration
// ============================================================================

enum class ENetChannel : uint8_t {
    RELIABLE_ORDERED = 0,      // Auth, inventory, merchants
    UNRELIABLE_SEQUENCED = 1,  // Movement, rotation, shooting
    RELIABLE_UNORDERED = 2,    // Chat, notifications
};

// ============================================================================
// Data Structures
// ============================================================================

struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
};

struct Quaternion {
    float x, y, z, w;

    Quaternion() : x(0), y(0), z(0), w(1) {}
    Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
};

// Player state for network sync
struct PlayerState {
    uint32_t playerId;
    Vector3 position;
    Vector3 velocity;
    float yaw;           // Horizontal rotation
    float pitch;         // Vertical rotation (camera)
    uint8_t weaponIndex; // Current weapon
    uint16_t health;
    uint16_t maxHealth;
    uint8_t flags;       // Crouching, jumping, etc.
};

// Weapon fire event
struct WeaponFireEvent {
    uint32_t playerId;
    Vector3 origin;
    Vector3 direction;
    uint16_t weaponId;
    uint32_t timestamp;
};

// Hit result from server
struct HitResult {
    uint32_t shooterId;
    uint32_t victimId;
    uint16_t damage;
    uint8_t bodyPart;  // Head, chest, limbs
    Vector3 hitPosition;
};

// Loot item on ground
struct LootItem {
    uint32_t lootId;
    uint16_t itemId;
    Vector3 position;
    uint16_t quantity;
    bool foundInRaid;
};

// ============================================================================
// Packet Serialization Helpers
// ============================================================================

class PacketBuilder {
public:
    PacketBuilder(PacketType type) {
        data.push_back(static_cast<uint8_t>(type));
    }

    void writeUInt8(uint8_t value) {
        data.push_back(value);
    }

    void writeUInt16(uint16_t value) {
        data.push_back(value & 0xFF);
        data.push_back((value >> 8) & 0xFF);
    }

    void writeUInt32(uint32_t value) {
        data.push_back(value & 0xFF);
        data.push_back((value >> 8) & 0xFF);
        data.push_back((value >> 16) & 0xFF);
        data.push_back((value >> 24) & 0xFF);
    }

    void writeUInt64(uint64_t value) {
        for (int i = 0; i < 8; i++) {
            data.push_back((value >> (i * 8)) & 0xFF);
        }
    }

    void writeFloat(float value) {
        uint32_t temp;
        std::memcpy(&temp, &value, sizeof(float));
        writeUInt32(temp);
    }

    void writeString(const std::string& str) {
        writeUInt16(static_cast<uint16_t>(str.size()));
        data.insert(data.end(), str.begin(), str.end());
    }

    void writeVector3(const Vector3& vec) {
        writeFloat(vec.x);
        writeFloat(vec.y);
        writeFloat(vec.z);
    }

    void writePlayerState(const PlayerState& state) {
        writeUInt32(state.playerId);
        writeVector3(state.position);
        writeVector3(state.velocity);
        writeFloat(state.yaw);
        writeFloat(state.pitch);
        writeUInt8(state.weaponIndex);
        writeUInt16(state.health);
        writeUInt16(state.maxHealth);
        writeUInt8(state.flags);
    }

    const uint8_t* getData() const { return data.data(); }
    size_t getSize() const { return data.size(); }

private:
    std::vector<uint8_t> data;
};

class PacketReader {
public:
    PacketReader(const uint8_t* data, size_t size)
        : data(data), size(size), offset(0) {}

    PacketType getType() const {
        return static_cast<PacketType>(data[0]);
    }

    uint8_t readUInt8() {
        if (offset + 1 > size) return 0;
        return data[offset++];
    }

    uint16_t readUInt16() {
        if (offset + 2 > size) return 0;
        uint16_t value = data[offset] | (data[offset + 1] << 8);
        offset += 2;
        return value;
    }

    uint32_t readUInt32() {
        if (offset + 4 > size) return 0;
        uint32_t value = data[offset] | (data[offset + 1] << 8) |
                        (data[offset + 2] << 16) | (data[offset + 3] << 24);
        offset += 4;
        return value;
    }

    uint64_t readUInt64() {
        if (offset + 8 > size) return 0;
        uint64_t value = 0;
        for (int i = 0; i < 8; i++) {
            value |= static_cast<uint64_t>(data[offset + i]) << (i * 8);
        }
        offset += 8;
        return value;
    }

    float readFloat() {
        uint32_t temp = readUInt32();
        float value;
        std::memcpy(&value, &temp, sizeof(float));
        return value;
    }

    std::string readString() {
        uint16_t length = readUInt16();
        if (offset + length > size) return "";
        std::string str(reinterpret_cast<const char*>(data + offset), length);
        offset += length;
        return str;
    }

    Vector3 readVector3() {
        return Vector3(readFloat(), readFloat(), readFloat());
    }

    PlayerState readPlayerState() {
        PlayerState state;
        state.playerId = readUInt32();
        state.position = readVector3();
        state.velocity = readVector3();
        state.yaw = readFloat();
        state.pitch = readFloat();
        state.weaponIndex = readUInt8();
        state.health = readUInt16();
        state.maxHealth = readUInt16();
        state.flags = readUInt8();
        return state;
    }

    bool hasMoreData() const {
        return offset < size;
    }

    size_t getOffset() const { return offset; }

private:
    const uint8_t* data;
    size_t size;
    size_t offset;
};

} // namespace TDS
