# Extraction Shooter - Raylib 3D Edition

A complete multiplayer 3D looter extraction shooter built with **raylib**, **raygui**, and featuring **NAT punchthrough** for peer-to-peer connectivity.

## ⚠️ IMPORTANT: Download Dependencies First!

Before building, **you MUST run**:
```cmd
download_dependencies.bat
```

This downloads raylib, raygui, and ENet. Without this, the project will not compile!

See [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) for detailed build steps.

---

## Features

### Core Game Features
- **3D Graphics** - Full 3D rendering using raylib
- **Third-Person Camera** - Smooth camera following player
- **Character Animations** - Complete animation system (idle, walk, run, shoot, reload, die, loot)
- **Multiplayer** - Authoritative server architecture with client prediction
- **NAT Punchthrough** - UDP hole punching for P2P connections
- **Body Looting** - Loot corpses of fallen players
- **Inventory System** - Grid-based stash and loadout management
- **Extraction Mechanics** - Extract from raids to keep your loot

### Technical Features
- **raylib 5.0** - Modern 3D graphics library
- **raygui** - Immediate mode GUI for all UI elements
- **TCP Networking** - Reliable game server communication
- **UDP NAT Punchthrough** - Direct peer connections through firewalls
- **Client-Server Architecture** - Authoritative server prevents cheating
- **Animation State Machine** - Smooth animation transitions
- **Model Loading** - Support for glTF, OBJ, IQM formats

## Project Structure

```
TDS/
├── download_dependencies.bat        # Downloads raylib, raygui, ENet
├── ExtractionShooter.sln           # Visual Studio solution
├── ExtractionShooterRaylib.vcxproj # Raylib client project
├── ExtractionShooterServer.vcxproj # Game server project
├── NatPunchServer.vcxproj          # NAT punchthrough server
│
├── dependencies/                    # Created by download script
│   ├── raylib/                     # raylib 5.0
│   ├── raygui/                     # raygui header
│   └── enet/                       # NAT punchthrough library
│
├── resources/                       # Game assets
│   ├── models/                     # 3D character models (.glb, .obj)
│   ├── animations/                 # Animation files
│   ├── textures/                   # Textures and sprites
│   └── sounds/                     # Audio files
│
└── src/
    ├── client/
    │   ├── RaylibMain.cpp          # Raylib client entry point
    │   ├── RaylibGameClient.h      # Main game client
    │   ├── animation/
    │   │   └── AnimationController.h
    │   └── network/
    │       └── NetworkClient.h
    │
    ├── server/
    │   ├── main.cpp                # Server entry point
    │   └── managers/               # Server-side managers
    │
    ├── natpunch/
    │   ├── NatPunchServer.h        # NAT server
    │   ├── NatPunchServerMain.cpp
    │   └── NatPunchClient.h        # NAT client integration
    │
    └── common/
        ├── NetworkProtocol.h       # Shared packet definitions
        ├── DataStructures.h        # Game data structures
        ├── ItemDatabase.h          # Item definitions
        └── CorpseSystem.h          # Body looting system
```

## Getting Started

### Step 1: Download Dependencies

Run the batch file to download all required libraries:

```cmd
download_dependencies.bat
```

This will download:
- raylib 5.0 (3D graphics engine)
- raygui (UI library)
- ENet (NAT punchthrough networking)

### Step 2: Build the Solution

1. Open `ExtractionShooter.sln` in Visual Studio 2022
2. Select configuration: `Debug` or `Release`
3. Select platform: `x64` (recommended) or `Win32`
4. Build → Build Solution (Ctrl+Shift+B)

This will build:
- **ExtractionShooterServer** - Game server
- **ExtractionShooterRaylib** - Raylib 3D client
- **NatPunchServer** - NAT punchthrough server (optional)

### Step 3: Run the Game

#### Start Game Server
```cmd
cd x64\Release
ExtractionShooterServer.exe
```
Server will start on port **7777**

#### (Optional) Start NAT Punchthrough Server
```cmd
NatPunchServer.exe
```
NAT server will start on UDP port **3478**

#### Start Client
```cmd
ExtractionShooterRaylib.exe
```

Or with NAT punchthrough:
```cmd
ExtractionShooterRaylib.exe --nat --server 127.0.0.1 --port 7777
```

## Controls

### Login Screen
- **Type** - Enter username and password
- **TAB** - Switch between input fields
- **Click buttons** - Login or Register

### Main Menu
- **Click buttons** - Navigate menus
- **ESC** - Go back

### In-Game (Raid)
- **W/A/S/D** - Move (forward, left, back, right)
- **Left Shift** - Toggle sprint
- **C** - Toggle crouch
- **Left Mouse** - Shoot weapon
- **R** - Reload
- **F** - Loot nearby corpse
- **E** - Extract (when near extraction zone)
- **TAB** - Toggle inventory
- **ESC** - Exit to menu

### Camera
- **Mouse Movement** - Rotate camera
- Automatic third-person following

## Animation System

### Supported Animations
- **Idle** - Standing still
- **Walk** - Normal movement
- **Run** - Sprinting
- **Crouch** - Crouched stance
- **Jump** - Jumping (future)
- **Shoot** - Firing weapon
- **Reload** - Reloading weapon
- **Melee** - Melee attack
- **Hit** - Taking damage
- **Die** - Death animation
- **Loot** - Looting corpse

### Animation State Machine
Animations automatically transition based on player state:
- Death overrides all other animations
- Action animations (shoot, reload, loot) have priority
- Movement animations blend based on speed

### Adding Custom Models

1. Export your model with animations to `.glb` (glTF) format
2. Place model in `resources/models/player.glb`
3. Place animations in `resources/animations/player_anims.glb`
4. Adjust frame ranges in `AnimationController.h` → `setupDefaultClips()`

Example frame ranges:
```cpp
m_clips[AnimationType::IDLE] = AnimationClip("Idle", 0, 60, 1.0f, true);
m_clips[AnimationType::WALK] = AnimationClip("Walk", 61, 91, 1.0f, true);
m_clips[AnimationType::RUN] = AnimationClip("Run", 92, 112, 1.5f, true);
// ... etc
```

## Corpse & Looting System

### How It Works
1. When a player dies, a corpse spawns at their location
2. Corpse contains all items the player had equipped
3. Other players can approach and press **F** to loot
4. Items can be taken individually or all at once
5. Corpses remain for 30 minutes, then despawn

### Loot Distance
- Must be within **3 meters** to loot a corpse
- Corpses are marked with a gold cube indicator

### Server Authority
- Server validates all loot actions
- Prevents duplication exploits
- Tracks who looted each corpse

## NAT Punchthrough

### Why NAT Punchthrough?
Allows players behind routers/firewalls to connect directly for:
- Voice chat
- Reduced latency
- Peer-to-peer data sharing

### Setup
1. Run `NatPunchServer.exe` on a public server
2. Clients connect with `--nat` flag
3. Clients register with NAT server
4. NAT server facilitates UDP hole punching

### Architecture
```
[Client A] <--TCP--> [Game Server] <--TCP--> [Client B]
     |                                           |
     +--------<--UDP (P2P) via NAT server-->-----+
```

### Usage
```cmd
# Run NAT server on public IP
NatPunchServer.exe 3478

# Clients connect to NAT server
ExtractionShooterRaylib.exe --nat
```

## Network Protocol

### TCP (Game Server)
- Port: 7777
- Reliable packet delivery
- Authentication, gameplay, inventory

### UDP (NAT Punchthrough)
- Port: 3478
- Fast, unreliable
- Voice chat, supplementary data

### Packet Types
- **0-99**: Authentication (login, register)
- **100-199**: Lobby system
- **200-299**: Friends
- **300-399**: Match/gameplay
- **320-325**: Corpse system
- **400-499**: Merchants
- **500-599**: Player data

## Performance

### Recommended Specs
- **CPU**: Intel Core i5 / AMD Ryzen 5
- **GPU**: NVIDIA GTX 1050 / AMD RX 560
- **RAM**: 4 GB
- **OS**: Windows 10/11 64-bit

### Graphics Settings
Currently running at:
- **Resolution**: 1280x720
- **FPS Target**: 60 FPS
- **Render Distance**: Configurable

## Development

### Adding New Features

#### Add New Animation
1. Export animation to glTF
2. Add to `AnimationType` enum
3. Define frame range in `setupDefaultClips()`
4. Update `PlayerAnimationStateMachine`

#### Add New Item Type
1. Define in `ItemDatabase.h`
2. Add item definition with stats
3. Update loot tables
4. Add merchant pricing

#### Add New Packet
1. Define packet struct in `NetworkProtocol.h`
2. Add handler in server
3. Add sender/receiver in client
4. Update packet type enum

### Debugging

Enable verbose logging:
```cpp
SetTraceLogLevel(LOG_DEBUG);
```

Show debug info:
- FPS is shown by default
- Position displayed in HUD
- Console output for network events

## Troubleshooting

### "Failed to load model"
- Check `resources/models/player.glb` exists
- Verify model format (glTF 2.0)
- Check console for specific error

### "Failed to connect to server"
- Ensure server is running first
- Check firewall settings
- Verify port 7777 is not blocked

### "NAT punchthrough failed"
- NAT server must be on public IP
- Check UDP port 3478 is open
- Ensure clients can reach NAT server

### Low FPS
- Reduce render distance
- Lower model polygon count
- Disable shadows (if implemented)
- Use Release build instead of Debug

## Building from Source

### Requirements
- Visual Studio 2022 or later
- Windows SDK 10.0
- C++17 compiler
- raylib 5.0 (auto-downloaded)

### Build Steps
```cmd
# Download dependencies
download_dependencies.bat

# Build with MSBuild
msbuild ExtractionShooter.sln /p:Configuration=Release /p:Platform=x64

# Or use Visual Studio
# Open ExtractionShooter.sln
# Build → Build Solution
```

### Output
Executables will be in:
- `x64\Release\ExtractionShooterRaylib.exe`
- `x64\Release\ExtractionShooterServer.exe`
- `x64\Release\NatPunchServer.exe`

## Multiplayer Testing

### Local Testing
1. Start server: `ExtractionShooterServer.exe`
2. Start client 1: `ExtractionShooterRaylib.exe`
3. Start client 2: `ExtractionShooterRaylib.exe`
4. Both clients connect to localhost:7777

### LAN Testing
1. Start server on host PC
2. Note host IP address (e.g., 192.168.1.100)
3. On client PC: `ExtractionShooterRaylib.exe --server 192.168.1.100`

### Internet Testing
1. Port forward 7777 (TCP) on host router
2. Note public IP
3. On client: `ExtractionShooterRaylib.exe --server <public_ip>`
4. Optional: Use NAT punchthrough for better connectivity

## Future Enhancements

### Planned Features
- [ ] More maps
- [ ] AI enemies with pathfinding
- [ ] Weapon attachments
- [ ] Vehicle system
- [ ] Voice chat integration
- [ ] Skill progression
- [ ] Quest system
- [ ] Hideout customization
- [ ] Trading system
- [ ] Clan/guild system

### Technical Improvements
- [ ] Client prediction
- [ ] Server reconciliation
- [ ] Entity interpolation
- [ ] LOD (Level of Detail)
- [ ] Occlusion culling
- [ ] Anti-cheat improvements
- [ ] Database backend
- [ ] Encrypted traffic

## Contributing

This is a learning/educational project. Feel free to:
- Fork and modify
- Add new features
- Fix bugs
- Improve documentation

## License

Educational/personal project - see LICENSE file

## Credits

- **raylib** - Ramon Santamaria (@raysan5)
- **raygui** - raylib community
- **ENet** - Lee Salzman
- Built with C++ and Visual Studio

## Links

- raylib: https://www.raylib.com/
- raygui: https://github.com/raysan5/raygui
- ENet: http://enet.bespin.org/

---

**Last Updated**: 2025-11-15
**Version**: 2.0 (Raylib Edition)
**Status**: ✅ Fully Functional

Enjoy the game!
