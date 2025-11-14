# TDS - Tarkov-Style Looter Shooter (Raylib Edition)

A multiplayer extraction-based FPS game inspired by Escape from Tarkov, completely rebuilt with **Raylib** for modern cross-platform 3D rendering and **ENet** for reliable UDP networking.

## 🎮 Features

### Core Gameplay
- **Extraction Shooter Mechanics** - High-stakes raids where death means losing gear
- **Persistent Progression** - Keep your stash between raids
- **Looting System** - 50+ items with rarity tiers
- **Weapon System** - Realistic ballistics and weapon handling
- **Inventory Management** - Grid-based Tetris-style inventory
- **Dynamic Economy** - 5 merchants with varying prices

### Multiplayer
- **Client-Server Architecture** - Authoritative server prevents cheating
- **UDP Networking (ENet)** - Low-latency with reliability layer
- **NAT Punchthrough** - STUN-based P2P hole punching
- **Client Prediction** - Smooth movement despite network latency
- **Lag Compensation** - Fair hit registration

### Technical
- **Raylib 5.0** - Modern OpenGL rendering, cross-platform
- **3D FPS Camera** - Smooth mouse look and WASD movement
- **PBR Materials** - Physically-based rendering
- **3D Audio** - Positional sound with Raylib audio
- **SQLite Database** - Persistent player data
- **CMake Build System** - Easy cross-platform compilation

## 📁 Project Structure

```
TDS/
├── CMakeLists.txt              # Main build configuration
├── BUILD.md                    # Build instructions
├── RAYLIB_ARCHITECTURE.md      # Technical architecture
├── download_dependencies.sh    # Dependency downloader
│
├── src/
│   ├── common/                 # Shared code
│   │   ├── Protocol.h/cpp      # Network protocol
│   │   ├── Items.h/cpp         # Item database (50+ items)
│   │   ├── Math.h/cpp          # Math utilities
│   │   └── Compression.h/cpp   # Data compression
│   │
│   ├── client/                 # Raylib game client
│   │   ├── main.cpp            # Entry point
│   │   ├── Game.h/cpp          # Main game class
│   │   ├── Rendering/          # 3D rendering with Raylib
│   │   ├── Network/            # ENet client + STUN
│   │   ├── Gameplay/           # Player, weapons, inventory
│   │   ├── UI/                 # Menus and HUD
│   │   └── Audio/              # Sound manager
│   │
│   └── server/                 # Authoritative server
│       ├── main.cpp            # Entry point
│       ├── Server.h/cpp        # Main server class
│       ├── Network/            # ENet server + STUN
│       ├── Database/           # SQLite wrapper
│       ├── Managers/           # Auth, Match, Loot, Merchants
│       └── Gameplay/           # Server-side game logic
│
├── external/                   # Third-party dependencies
│   ├── raylib/                 # Fetched by CMake
│   ├── enet/                   # UDP networking
│   └── sqlite/                 # Database
│
└── assets/                     # Game assets
    ├── models/                 # 3D models (.obj, .gltf)
    ├── textures/               # Textures and materials
    ├── sounds/                 # Sound effects (.ogg)
    └── music/                  # Background music
```

## 🚀 Quick Start

### 1. Download Dependencies
```bash
bash download_dependencies.sh
```

### 2. Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### 3. Run

**Start Server:**
```bash
./build/TDS_Server
```

**Start Client (in another terminal):**
```bash
./build/TDS_Client
```

### 4. Play
1. Create account or login
2. Navigate to lobby
3. Ready up and start queue
4. Spawn into raid
5. Loot, survive, extract!

See **BUILD.md** for detailed build instructions.

## 🎯 Game Modes

### Raid Mode
- 12-16 players spawn on map
- Loot containers and enemies
- Extract to keep gear
- Die and lose everything

### Stash Management
- Store items between raids
- Organize inventory
- Sell to merchants
- Buy new gear

### Economy
5 Merchants with different specialties:
- **Fence** - Buys/sells everything (worst prices)
- **Prapor** - Weapons & ammo specialist
- **Therapist** - Medical supplies
- **Peacekeeper** - Western gear (expensive)
- **Ragman** - Armor & clothing

## 🔧 Technology Stack

| Component | Technology |
|-----------|------------|
| **Rendering** | Raylib 5.0 (OpenGL) |
| **Networking** | ENet 1.3.17 (UDP + reliability) |
| **Database** | SQLite 3.45 |
| **Language** | C++17 |
| **Build** | CMake 3.15+ |
| **Platforms** | Linux, Windows, macOS |

## 🌐 Networking

### Protocol
- **ENet Channels:**
  - Channel 0: Reliable ordered (auth, inventory, merchants)
  - Channel 1: Unreliable sequenced (movement, shooting)
  - Channel 2: Reliable unordered (chat, notifications)

### NAT Traversal
- STUN client/server for NAT type detection
- UDP hole punching for P2P connections
- Fallback to server relay if needed

### Packet Compression
- Delta encoding for positions
- Quantized floats for rotations
- RLE compression for inventory

## 📊 Performance Targets

- **Client FPS:** 60-144 FPS on mid-range hardware
- **Server Tick Rate:** 60 Hz
- **Network Update Rate:** 20 Hz (movement), 60 Hz (combat)
- **Max Players:** 12-16 per raid
- **Concurrent Matches:** 10+ on dedicated server

## 🎨 Assets

### Sources for Free Assets
- **Models:** Kenney.nl, Quaternius, Sketchfab (CC0)
- **Sounds:** Freesound.org, OpenGameArt.org
- **Textures:** PolyHaven, AmbientCG
- **Music:** ccMixter, Incompetech

Place assets in respective folders and build will copy them.

## 🛠️ Development

### Prerequisites
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2019+)
- CMake 3.15+
- OpenGL support
- Git

### Building from Source
```bash
git clone https://github.com/The404Studios/TDS.git
cd TDS
bash download_dependencies.sh
cmake -B build
cmake --build build
```

### IDE Setup
- **VS Code:** Install C/C++ and CMake Tools extensions
- **CLion:** Open folder, auto-detects CMake
- **Visual Studio:** File → Open → CMake

## 📖 Documentation

- **BUILD.md** - Detailed build instructions
- **RAYLIB_ARCHITECTURE.md** - Technical architecture
- **NETWORKING.md** - Network protocol details
- **ASSETS.md** - Asset guidelines

## 🤝 Contributing

We welcome contributions! Areas needing help:
- 3D models and animations
- Sound effects and music
- Map design
- Gameplay balancing
- Bug fixes and optimization

## 📝 License

This is an educational project. See LICENSE for details.

## 🎮 Controls

### In-Game
- **WASD** - Movement
- **Mouse** - Look around
- **Left Click** - Shoot
- **R** - Reload
- **Tab** - Inventory
- **E** - Interact / Extract
- **Esc** - Menu

### Menus
- **Mouse** - Navigate
- **Enter** - Confirm
- **Esc** - Back

## 🐛 Known Issues

- [ ] Combat system needs server validation
- [ ] Loot spawning needs optimization
- [ ] UI needs proper font rendering
- [ ] AI pathfinding incomplete
- [ ] Asset placeholders need replacement

## 🗺️ Roadmap

### v0.1 (Current)
- [x] Basic Raylib client
- [x] ENet networking
- [ ] FPS movement and camera
- [ ] Simple shooting mechanics

### v0.2
- [ ] Full inventory system
- [ ] Merchant UI
- [ ] Item spawning
- [ ] Basic AI enemies

### v0.3
- [ ] Multiple maps
- [ ] Weapon variety
- [ ] Sound effects
- [ ] Visual polish

### v1.0
- [ ] Complete gameplay loop
- [ ] NAT punchthrough
- [ ] Anti-cheat
- [ ] Full asset pack

## 💬 Community

- **GitHub Issues:** Bug reports and features
- **Discord:** (Coming soon)
- **Wiki:** Game mechanics and guides

## 🙏 Credits

- Inspired by Escape from Tarkov
- Built with Raylib by Ramon Santamaria
- ENet by Lee Salzman
- Community contributors

---

**Made with ❤️ using Raylib and C++**

*Last Updated: 2025-11-14*
