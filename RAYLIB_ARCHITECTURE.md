# Raylib Tarkov Looter Shooter - Architecture

## Overview

Complete rewrite of the extraction shooter using **Raylib** for cross-platform 3D rendering, with enhanced networking including NAT traversal.

## Technology Stack

### Client
- **Raylib 5.0** - 3D rendering, audio, input, and UI
- **enet** - UDP networking with reliability
- **STUN client** - NAT traversal for P2P connections
- **C++17** - Modern C++ features

### Server
- **enet** - UDP server with reliability layer
- **STUN server integration** - NAT punchthrough
- **SQLite** - Database for persistence (upgrade from text files)
- **C++17** - Authoritative game logic

## Architecture

```
TDS/
├── CMakeLists.txt              # Cross-platform build system
├── external/                   # Third-party dependencies
│   ├── raylib/                 # Raylib 5.0
│   ├── enet/                   # ENet networking
│   └── sqlite/                 # SQLite database
│
├── assets/                     # Game assets
│   ├── models/                 # 3D models (.obj, .gltf)
│   │   ├── weapons/           # Weapon models
│   │   ├── characters/        # Player models
│   │   ├── items/             # Loot models
│   │   └── maps/              # Map geometry
│   ├── textures/              # Textures and materials
│   ├── sounds/                # Sound effects
│   │   ├── weapons/           # Gunshots, reloads
│   │   ├── footsteps/         # Movement sounds
│   │   └── ambient/           # Environment
│   └── music/                 # Background music
│
├── src/
│   ├── common/                # Shared between client/server
│   │   ├── Protocol.h         # Network protocol
│   │   ├── Items.h            # Item definitions
│   │   ├── Math.h             # Math utilities
│   │   └── Compression.h      # Data compression
│   │
│   ├── client/                # Raylib game client
│   │   ├── main.cpp           # Client entry point
│   │   ├── Game.h/cpp         # Main game class
│   │   ├── Network/
│   │   │   ├── NetworkClient.h/cpp    # ENet client
│   │   │   └── STUNClient.h/cpp       # NAT traversal
│   │   ├── Rendering/
│   │   │   ├── Renderer.h/cpp         # Raylib renderer
│   │   │   ├── Camera.h/cpp           # FPS camera
│   │   │   ├── ModelManager.h/cpp     # 3D model loading
│   │   │   └── ParticleSystem.h/cpp   # Effects
│   │   ├── Gameplay/
│   │   │   ├── Player.h/cpp           # Local player
│   │   │   ├── RemotePlayer.h/cpp     # Other players
│   │   │   ├── Weapon.h/cpp           # Weapon system
│   │   │   ├── Inventory.h/cpp        # Inventory management
│   │   │   └── LootSpawn.h/cpp        # Loot visualization
│   │   ├── UI/
│   │   │   ├── UIManager.h/cpp        # UI state machine
│   │   │   ├── MainMenu.h/cpp         # Main menu
│   │   │   ├── HUD.h/cpp              # In-game HUD
│   │   │   ├── InventoryUI.h/cpp      # Inventory screen
│   │   │   └── LoginUI.h/cpp          # Authentication
│   │   └── Audio/
│   │       └── AudioManager.h/cpp     # Sound management
│   │
│   └── server/               # Authoritative server
│       ├── main.cpp          # Server entry point
│       ├── Server.h/cpp      # Main server class
│       ├── Network/
│       │   ├── NetworkServer.h/cpp    # ENet server
│       │   ├── STUNServer.h/cpp       # NAT coordination
│       │   └── PacketHandler.h/cpp    # Packet processing
│       ├── Database/
│       │   └── Database.h/cpp         # SQLite wrapper
│       ├── Managers/
│       │   ├── AuthManager.h/cpp      # Authentication
│       │   ├── MatchManager.h/cpp     # Match creation
│       │   ├── PlayerManager.h/cpp    # Player state
│       │   ├── LootManager.h/cpp      # Loot spawning
│       │   └── MerchantManager.h/cpp  # Economy
│       └── Gameplay/
│           ├── GameWorld.h/cpp        # World simulation
│           ├── AIController.h/cpp     # AI enemies
│           └── PhysicsWorld.h/cpp     # Server physics
│
└── docs/
    ├── NETWORKING.md         # Network protocol
    ├── ASSETS.md             # Asset guidelines
    └── BUILD.md              # Build instructions
```

## Key Features

### 1. Raylib Rendering
- **3D FPS View** - First-person camera with smooth movement
- **PBR Materials** - Physically-based rendering
- **Dynamic Lighting** - Real-time shadows and lights
- **Particle Effects** - Muzzle flash, blood, smoke
- **Post-processing** - Bloom, color grading
- **Weapon Animations** - Procedural weapon sway/recoil

### 2. Enhanced Networking
- **ENet Protocol** - UDP with reliable packets
- **NAT Punchthrough** - STUN-based hole punching
- **Client Prediction** - Smooth movement
- **Server Reconciliation** - Anti-cheat validation
- **Lag Compensation** - Hit registration
- **Compression** - Bandwidth optimization

### 3. Gameplay Systems
- **Weapon System** - Raycasting for bullets, recoil patterns
- **Inventory System** - Grid-based inventory with Raylib UI
- **Loot System** - 3D item spawns with highlights
- **AI Enemies** - Server-controlled scavs with pathfinding
- **Health System** - Body part damage, healing
- **Extraction** - Dynamic extraction points

### 4. Audio
- **3D Positional Audio** - Raylib 3D sound
- **Weapon Sounds** - Gunshots, reloads, handling
- **Environmental Audio** - Wind, rain, ambient
- **Footsteps** - Surface-based footstep sounds
- **Music** - Menu and in-game soundtrack

## Network Protocol (ENet)

### Connection Flow
```
1. Client connects to server TCP:7777
2. Client sends STUN request for NAT type
3. Server coordinates hole punching
4. Client/Server establish UDP connection via ENet
5. Authentication via reliable channel
6. Game packets via unreliable channel (with sequencing)
```

### Packet Types
- **Reliable Ordered** - Login, inventory, transactions
- **Unreliable Sequenced** - Movement, rotation, shooting
- **Reliable Unordered** - Chat messages

### Packet Structure (ENet handles framing)
```cpp
struct GamePacket {
    uint8_t type;           // PacketType enum
    uint32_t sequence;      // For ordering unreliable packets
    uint64_t timestamp;     // Client timestamp
    // Payload (varies by type)
};
```

## Asset Pipeline

### Models
- **Format**: GLTF 2.0 (preferred) or OBJ
- **Poly Count**: LOD system (High: 5k-15k, Low: 500-2k)
- **Textures**: PBR (albedo, normal, metallic, roughness)
- **Sources**:
  - Sketchfab (CC0/CC-BY)
  - Kenney.nl (free game assets)
  - Quaternius (free low-poly)

### Sounds
- **Format**: OGG Vorbis (compressed) or WAV
- **Sample Rate**: 44.1kHz
- **Sources**:
  - Freesound.org (CC0/CC-BY)
  - OpenGameArt.org
  - BBC Sound Effects (free)

### Textures
- **Resolution**: 2048x2048 max (with mipmaps)
- **Format**: PNG for UI, compressed DDS for 3D
- **PBR Workflow**: Roughness/Metallic

## Build System (CMake)

### Platforms
- **Linux** (primary development)
- **Windows** (cross-compile or native)
- **macOS** (Apple Silicon + Intel)

### Dependencies
- Raylib (vendored)
- ENet (vendored)
- SQLite (vendored)
- OpenSSL (system, optional for HTTPS)

### Build Commands
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build -j$(nproc)

# Run server
./build/server/TDS_Server

# Run client
./build/client/TDS_Client
```

## Database Schema (SQLite)

### Tables
```sql
CREATE TABLE accounts (
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE,
    password_hash TEXT,
    created_at TIMESTAMP,
    last_login TIMESTAMP
);

CREATE TABLE players (
    id INTEGER PRIMARY KEY,
    account_id INTEGER,
    roubles INTEGER,
    level INTEGER,
    experience INTEGER,
    stats JSON,  -- kills, deaths, raids, etc.
    FOREIGN KEY(account_id) REFERENCES accounts(id)
);

CREATE TABLE inventory (
    id INTEGER PRIMARY KEY,
    player_id INTEGER,
    item_id INTEGER,
    quantity INTEGER,
    position_x INTEGER,
    position_y INTEGER,
    found_in_raid BOOLEAN,
    FOREIGN KEY(player_id) REFERENCES players(id)
);

CREATE TABLE friends (
    id INTEGER PRIMARY KEY,
    player1_id INTEGER,
    player2_id INTEGER,
    status TEXT,  -- pending, accepted
    created_at TIMESTAMP,
    FOREIGN KEY(player1_id) REFERENCES players(id),
    FOREIGN KEY(player2_id) REFERENCES players(id)
);
```

## Implementation Phases

### Phase 1: Foundation (Week 1)
- [ ] Set up CMake build system
- [ ] Integrate Raylib
- [ ] Integrate ENet
- [ ] Create basic client window
- [ ] Create basic server

### Phase 2: Networking (Week 1-2)
- [ ] Implement ENet client/server
- [ ] Add STUN NAT traversal
- [ ] Create protocol with ENet channels
- [ ] Test client-server communication

### Phase 3: Core Gameplay (Week 2-3)
- [ ] FPS camera controller
- [ ] Player movement with Raylib
- [ ] Weapon system (raycasting)
- [ ] Inventory system (grid UI)
- [ ] Health and damage

### Phase 4: Visuals & Audio (Week 3-4)
- [ ] Load and render 3D models
- [ ] PBR materials and lighting
- [ ] Particle effects (muzzle flash, etc.)
- [ ] 3D audio system
- [ ] UI with Raylib GUI

### Phase 5: Multiplayer (Week 4-5)
- [ ] Player synchronization
- [ ] Client prediction
- [ ] Server reconciliation
- [ ] Lag compensation
- [ ] Anti-cheat validation

### Phase 6: Content (Week 5-6)
- [ ] Add game assets (models, textures, sounds)
- [ ] AI enemies with pathfinding
- [ ] Multiple maps
- [ ] Loot spawning
- [ ] Merchant system

### Phase 7: Polish (Week 6+)
- [ ] Optimization
- [ ] Bug fixes
- [ ] Balance tuning
- [ ] Documentation
- [ ] Testing

## Performance Targets

- **Client FPS**: 60+ FPS on mid-range hardware
- **Server Tick Rate**: 60 Hz
- **Network Update Rate**: 20 Hz (movement), 60 Hz (shooting)
- **Max Players per Match**: 12-16
- **Concurrent Server Matches**: 10+

## Security Considerations

- **Password Hashing**: bcrypt or argon2
- **Session Tokens**: Cryptographically secure random
- **Anti-Cheat**: Server validation of all actions
- **Rate Limiting**: Prevent packet flooding
- **SQL Injection**: Prepared statements only
- **Encryption**: Optional TLS for auth (over TCP), ENet compression for UDP

## Next Steps

1. Set up CMake project structure
2. Download and integrate Raylib
3. Create basic client window with Raylib
4. Create basic server with ENet
5. Establish client-server connection
6. Implement FPS camera and movement
7. Add weapon shooting with raycasting
8. Start integrating assets

---

**Author**: Claude Code
**Started**: 2025-11-14
**Target**: Full Raylib-based Tarkov looter shooter
