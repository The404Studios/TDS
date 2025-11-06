# EXTRACTION SHOOTER - Complete Rewrite Specification

## Project: Tarkov-Style Multiplayer Extraction Shooter

This document outlines the complete architecture for a professional multiplayer extraction shooter with:
- Authentication & Account System
- Lobby & Matchmaking
- Friend System
- Merchant & Economy
- Server-Side Authority
- Player Persistence

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                     CLIENT                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │   Login UI   │→ │  Lobby UI    │→ │   Game UI    │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│         ↓                  ↓                  ↓         │
│  ┌──────────────────────────────────────────────────┐  │
│  │         Network Client Manager                    │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
                            ↕ TCP/UDP
┌─────────────────────────────────────────────────────────┐
│                     SERVER                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │   Auth Mgr   │  │  Lobby Mgr   │  │  Match Mgr   │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐ │
│  │  Account DB  │  │  Friend Mgr  │  │ Merchant Mgr │ │
│  └──────────────┘  └──────────────┘  └──────────────┘ │
│  ┌──────────────────────────────────────────────────┐  │
│  │         Player Persistence Manager                │  │
│  └──────────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

---

## Phase 1: Foundation (Core Systems)

### 1.1 Network Protocol
- **Packet Types:**
  - `LOGIN_REQUEST` / `LOGIN_RESPONSE`
  - `REGISTER_REQUEST` / `REGISTER_RESPONSE`
  - `LOBBY_CREATE` / `LOBBY_JOIN` / `LOBBY_LEAVE`
  - `FRIEND_REQUEST` / `FRIEND_ACCEPT` / `FRIEND_REMOVE`
  - `MATCH_START` / `MATCH_END`
  - `PLAYER_MOVE` / `PLAYER_SHOOT` / `PLAYER_LOOT`
  - `MERCHANT_BUY` / `MERCHANT_SELL`

### 1.2 Data Structures
```cpp
struct Account {
    uint64_t accountId;
    string username;
    string passwordHash;
    uint64_t created;
    uint64_t lastLogin;
};

struct PlayerData {
    uint64_t accountId;
    int level;
    int roubles;
    vector<Item> stash;
    vector<Item> loadout;
    Stats stats;
};

struct Lobby {
    uint64_t lobbyId;
    uint64_t ownerId;
    vector<uint64_t> members;
    int maxPlayers;
    bool inQueue;
};

struct Match {
    uint64_t matchId;
    vector<uint64_t> players;
    string mapName;
    float startTime;
    bool active;
};
```

---

## Phase 2: Authentication System

### 2.1 Login Flow
```
Client                          Server
  │                               │
  ├─ LOGIN_REQUEST ──────────────→│
  │  (username, password)          │
  │                               ├─ Validate credentials
  │                               ├─ Check DB
  │                               ├─ Create session token
  │←─ LOGIN_RESPONSE ─────────────┤
  │  (success, token, accountId)  │
  │                               │
  ├─ All future packets ──────────→│
  │  include session token         │
```

### 2.2 Registration
- Username validation (3-16 chars, alphanumeric)
- Password hashing (SHA-256 + salt)
- Email verification (optional)
- Starting gear assignment

---

## Phase 3: Lobby & Social Systems

### 3.1 Lobby Features
- Create private/public lobby
- Invite friends to lobby
- Ready check system
- Lobby chat
- Kick/ban players (owner only)
- Queue for match (min 1 player, max 5)

### 3.2 Friend System
- Send friend request
- Accept/decline requests
- View friends list (online/offline status)
- Invite to lobby
- Remove friend
- Block user

---

## Phase 4: Merchant & Economy

### 4.1 Merchant Types
1. **Fence** - Buys/sells everything (low prices)
2. **Prapor** - Weapons & ammo specialist
3. **Therapist** - Medical supplies
4. **Peacekeeper** - Western gear (expensive)
5. **Ragman** - Armor & clothing

### 4.2 Trading
```cpp
// Buy from merchant
BuyRequest {
    merchantId,
    itemId,
    quantity
} → Check funds → Deduct roubles → Add to stash

// Sell to merchant
SellRequest {
    merchantId,
    itemStashIndex,
    quantity
} → Check item exists → Remove from stash → Add roubles
```

### 4.3 Pricing
- Base prices in item database
- Merchant markup: Fence (0.6x), Prapor (1.0x), Therapist (1.2x), etc.
- Found-in-Raid items sell for more

---

## Phase 5: Match & Extraction System

### 5.1 Matchmaking
```
Lobby (1-5 players)
  ↓
Queue for match
  ↓
Server creates Match instance
  ↓
All players spawn in same location
  ↓
30-minute raid timer starts
  ↓
Players loot, fight AI, fight each other
  ↓
Extract at extraction zones OR die
  ↓
Post-raid screen & loot transfer
```

### 5.2 Spawn Coordination
- Party members spawn within 50m of each other
- Random spawn selection from map spawn points
- Anti-spawn-camp timer (10s god mode)

### 5.3 Extraction Manager
- Per-map extraction zones
- Dynamic extractions (some require keys/payment)
- Shared extractions (all party members must be present)
- Solo extractions
- Extraction timer (7-10 seconds)

---

## Phase 6: Player Persistence Manager

### 6.1 Server Authority
```
Client sends action → Server validates → Server updates state → Server broadcasts
```

**Never trust client:**
- Validate all positions (anti-teleport)
- Validate all kills (anti-aimbot detection)
- Validate all loot pickups (anti-duping)
- Rate limit actions (anti-spam)

### 6.2 Data Synchronization
```
Player Action               Server Check               Database Update
─────────────────────────────────────────────────────────────────────
Extract successfully   →    Validate in extract zone  → Save stash + stats
Die in raid           →    Validate health <= 0      → Clear loadout
Buy from merchant     →    Validate funds            → Update roubles + stash
Sell item             →    Validate item exists      → Update roubles + stash
```

### 6.3 Anti-Cheat Measures
- Position validation (max speed check)
- Damage validation (weapon-based damage)
- Loot validation (proximity check)
- Rate limiting (max actions per second)
- Suspicious activity flagging

---

## Phase 7: Database Schema

```sql
-- Accounts
CREATE TABLE accounts (
    account_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(16) UNIQUE NOT NULL,
    password_hash CHAR(64) NOT NULL,
    email VARCHAR(255),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP
);

-- Player Data
CREATE TABLE players (
    account_id BIGINT PRIMARY KEY,
    level INT DEFAULT 1,
    experience INT DEFAULT 0,
    roubles INT DEFAULT 500000,
    raids_completed INT DEFAULT 0,
    raids_extracted INT DEFAULT 0,
    raids_died INT DEFAULT 0,
    kills INT DEFAULT 0,
    deaths INT DEFAULT 0,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id)
);

-- Stash
CREATE TABLE stash (
    stash_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    account_id BIGINT NOT NULL,
    item_id VARCHAR(50) NOT NULL,
    stack_size INT DEFAULT 1,
    found_in_raid BOOLEAN DEFAULT FALSE,
    current_ammo INT DEFAULT 0,
    durability INT DEFAULT 100,
    FOREIGN KEY (account_id) REFERENCES accounts(account_id)
);

-- Friends
CREATE TABLE friends (
    account_id BIGINT NOT NULL,
    friend_account_id BIGINT NOT NULL,
    status ENUM('pending', 'accepted', 'blocked') DEFAULT 'pending',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (account_id, friend_account_id),
    FOREIGN KEY (account_id) REFERENCES accounts(account_id),
    FOREIGN KEY (friend_account_id) REFERENCES accounts(account_id)
);
```

---

## Implementation Priority

### Immediate (Phase 1)
1. ✅ Fix compilation errors
2. ✅ Create clean network protocol
3. ✅ Implement basic client-server communication

### Short-term (Phase 2-3)
4. ⏳ Authentication system (login/register)
5. ⏳ Lobby creation and management
6. ⏳ Friend system

### Medium-term (Phase 4-5)
7. ⏳ Merchant system with buy/sell
8. ⏳ Matchmaking and spawn coordination
9. ⏳ Extraction manager

### Long-term (Phase 6-7)
10. ⏳ Server-side validation
11. ⏳ Anti-cheat systems
12. ⏳ Database persistence

---

## File Structure

```
TDS/
├── Common/                  # Shared between client and server
│   ├── NetworkProtocol.h    # Packet definitions
│   ├── DataStructures.h     # Account, PlayerData, etc.
│   └── ItemDatabase.h       # Item definitions
│
├── Client/
│   ├── main.cpp             # Client entry point
│   ├── LoginUI.h            # Login screen
│   ├── LobbyUI.h            # Lobby interface
│   ├── GameClient.h         # Main game client
│   ├── NetworkClient.h      # Client networking
│   └── Renderer.h           # OpenGL rendering
│
├── Server/
│   ├── ServerMain.cpp       # Server entry point
│   ├── AuthManager.h        # Authentication
│   ├── LobbyManager.h       # Lobby system
│   ├── MatchManager.h       # Match/raid management
│   ├── MerchantManager.h    # Economy system
│   ├── FriendManager.h      # Social system
│   ├── PersistenceManager.h # Database operations
│   └── NetworkServer.h      # Server networking
│
└── Database/
    └── schema.sql           # Database structure
```

---

## Next Steps

I'll now implement this architecture starting with:
1. Fix current compilation errors
2. Create clean network protocol
3. Build authentication system
4. Implement lobby system
5. Add friend system
6. Create merchant economy
7. Build server managers

This is professional-grade MMO architecture! Ready to proceed?
