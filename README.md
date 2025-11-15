# TDS - Tarkov-Style Looter Shooter

A multiplayer FPS looter shooter inspired by Escape from Tarkov, built with Raylib 5.0, ENet networking, and SQLite database.

## Features

### Client Features
- **Advanced FPS Controller**
  - Aim Down Sights (ADS) with smooth FOV transitions
  - Weapon motion system (bob, sway, recoil)
  - Dynamic FOV (75° normal, 85° sprint, 55° ADS)
  - Camera pitch recoil
  - Sprint mechanics

- **Rendering**
  - Raylib 5.0 3D rendering
  - Model loading and management
  - Particle system
  - 144 FPS target

- **UI System**
  - raygui integration
  - Dark military Tarkov-inspired theme
  - Main menu, login, HUD, inventory screens
  - Health bars, progress bars, interactive buttons

- **Audio**
  - Weapon sounds (AK-47, M4A1, pistols, sniper)
  - Footstep sounds (concrete, grass)
  - UI sounds (button clicks, hovers)

- **Networking**
  - ENet UDP networking
  - Client prediction and server reconciliation
  - STUN for NAT traversal

### Server Features
- **Database**
  - SQLite3 for persistence
  - User authentication (username/password)
  - Player progression (level, XP, money)
  - Inventory and stash storage

- **Game Systems**
  - 60Hz tick rate
  - Match management
  - Loot spawning
  - AI controller
  - Physics world
  - Merchant system

- **Network**
  - ENet UDP server
  - Packet handling
  - STUN server for NAT traversal

## Building

### Prerequisites
- CMake 3.15+
- C++17 compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Linux: X11 development libraries

```bash
# Ubuntu/Debian
sudo apt install libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
```

### Build Steps

```bash
# Clone the repository
git clone <repository-url>
cd TDS

# Generate assets (weapons, sounds, animations)
python3 generate_assets.py

# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build client and server
cmake --build . -j4

# Executables will be in build/
# - TDS_Client (2.7MB)
# - TDS_Server (1.8MB)
```

## Running

### Start the Server
```bash
cd build
./TDS_Server
# Server will listen on port 7777
```

### Start the Client
```bash
cd build
./TDS_Client
# Client will attempt to connect to 127.0.0.1:7777
```

## Project Structure

```
TDS/
├── assets/                  # Game assets
│   ├── models/             # 3D models (.obj)
│   ├── sounds/             # Audio files (.wav)
│   ├── animations/         # Animation files
│   └── textures/           # Textures
├── src/
│   ├── common/             # Shared code
│   │   ├── Protocol.h/cpp  # Network protocol
│   │   ├── Items.h/cpp     # Item database
│   │   └── Math.h/cpp      # Math utilities
│   ├── client/             # Client code
│   │   ├── Rendering/      # Camera, renderer, models
│   │   ├── Gameplay/       # Player, weapons, inventory
│   │   ├── UI/             # Menus, HUD
│   │   ├── Network/        # Network client
│   │   └── Audio/          # Audio manager
│   └── server/             # Server code
│       ├── Database/       # SQLite database
│       ├── Network/        # Network server
│       ├── Managers/       # Game managers
│       └── Gameplay/       # World, AI, physics
├── external/               # Dependencies
│   ├── enet/              # ENet 1.3.17
│   └── sqlite/            # SQLite 3.45
└── CMakeLists.txt         # Build configuration
```

## Controls (Client)

- **WASD** - Move
- **Mouse** - Look around
- **Left Mouse** - Fire weapon
- **Right Mouse** - Aim Down Sights (ADS)
- **Shift** - Sprint
- **R** - Reload
- **Tab** - Inventory
- **ESC** - Menu

## Technical Details

### Networking
- **Protocol**: UDP via ENet
- **Tick Rate**: 60Hz server, variable client
- **Features**: Client prediction, lag compensation, STUN NAT traversal

### Weapons
- AK-74, AK-74M (Assault Rifles)
- M4A1 (Assault Rifle)
- Glock 17 (Pistol)
- SVD (Sniper Rifle)
- SKS (DMR)

### Database Schema

**users table:**
- id (PRIMARY KEY)
- username (UNIQUE)
- password_hash
- created_at

**player_data table:**
- user_id (FOREIGN KEY)
- level
- experience
- money
- inventory (JSON)
- stash (JSON)

## Development Status

✅ Complete:
- Client rendering and FPS controller
- UI system with raygui
- Weapon system with ADS and recoil
- ENet networking (client/server)
- SQLite database integration
- Asset generation system
- Build system (CMake)

🚧 In Progress:
- Server game loop implementation
- Match management
- AI enemies
- Inventory persistence

## License

[Add your license here]

## Credits

- **Raylib** - 3D rendering engine
- **ENet** - UDP networking
- **SQLite** - Database
- **raygui** - Immediate mode GUI
