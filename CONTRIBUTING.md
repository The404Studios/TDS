# Contributing Guide

Thank you for your interest in contributing to Extraction Shooter! This guide will help you get started with modifying and improving the codebase.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Setup](#development-setup)
3. [Code Structure](#code-structure)
4. [Coding Standards](#coding-standards)
5. [Making Changes](#making-changes)
6. [Testing](#testing)
7. [Submitting Changes](#submitting-changes)
8. [Common Tasks](#common-tasks)

---

## Getting Started

### Prerequisites

Before contributing, ensure you have:
- ✅ Visual Studio 2022 (Community or Professional)
- ✅ Windows SDK 10.0
- ✅ Git for version control
- ✅ Basic C++17 knowledge
- ✅ Understanding of client-server architecture

### Recommended Knowledge

**For Client Development**:
- raylib API (https://www.raylib.com/cheatsheet/cheatsheet.html)
- raygui for UI (https://github.com/raysan5/raygui)
- 3D math (vectors, matrices, quaternions)
- Animation systems

**For Server Development**:
- Network programming (TCP/UDP)
- Authoritative server design
- Anti-cheat concepts
- File I/O and data persistence

---

## Development Setup

### 1. Clone the Repository

```bash
git clone <repository-url>
cd TDS
```

### 2. Download Dependencies

```cmd
download_dependencies.bat
```

This downloads:
- raylib 5.0
- raygui
- ENet (for NAT punchthrough)

### 3. Verify Environment

```cmd
verify_build_env.bat
```

Ensure all checks pass before proceeding.

### 4. Open in Visual Studio

```cmd
# Open the solution
start ExtractionShooter.sln
```

Set your preferred startup project:
- Right-click `ExtractionShooterRaylib` → Set as Startup Project (for client work)
- Right-click `ExtractionShooterServer` → Set as Startup Project (for server work)

### 5. Build and Test

```cmd
# Or use command line
msbuild ExtractionShooter.sln /p:Configuration=Debug /p:Platform=x64
```

---

## Code Structure

### Project Organization

```
src/
├── client/              # Raylib client code
│   ├── RaylibGameClient.h     # Main game client class
│   ├── RaylibMain.cpp         # Entry point
│   ├── PlaceholderModels.h    # Asset generation
│   ├── animation/             # Animation system
│   ├── audio/                 # Sound system
│   └── effects/               # Particle effects
│
├── server/              # Server code
│   ├── ServerMain.cpp         # Entry point
│   ├── NetworkServer.h        # TCP server
│   └── managers/              # Game logic
│       ├── AuthManager.h      # Authentication
│       ├── MatchManager.h     # Raid logic
│       └── ...
│
├── common/              # Shared between client and server
│   ├── NetworkProtocol.h      # Packet definitions
│   ├── DataStructures.h       # Shared data types
│   ├── ItemDatabase.h         # Item definitions
│   └── CorpseSystem.h         # Corpse/looting logic
│
└── natpunch/            # NAT punchthrough
    ├── NatPunchServer.h       # UDP hole punching server
    └── NatPunchClient.h       # Client integration
```

### Key Classes

**Client Side**:
- `RaylibGameClient` - Main game loop, rendering, input
- `AnimationController` - Character animations
- `SoundSystem` - Audio playback
- `ParticleSystem` - Visual effects
- `NetworkClient` - Server communication

**Server Side**:
- `NetworkServer` - TCP connection handling
- `AuthManager` - Login/registration
- `MatchManager` - Raid logic and game state
- `CorpseManager` - Body looting
- `MerchantManager` - Economy system

**Shared**:
- `NetworkProtocol` - All packet types (CRITICAL: sync client/server)
- `ItemDatabase` - Item definitions
- `DataStructures` - Player, match, item structs

---

## Coding Standards

### C++ Style Guide

**Naming Conventions**:
```cpp
// Classes: PascalCase
class PlayerController { };

// Member variables: m_ prefix + camelCase
class Player {
    int m_health;
    std::string m_username;
};

// Functions: camelCase
void updatePlayer(float deltaTime);

// Constants: UPPER_CASE
const int MAX_PLAYERS = 100;

// Enums: PascalCase for type, UPPER_CASE for values
enum class AnimationType {
    IDLE,
    WALK,
    RUN
};
```

**Include Order** (CRITICAL!):
```cpp
// Own header first
#include "MyClass.h"

// Winsock BEFORE Windows.h!
#include <winsock2.h>
#include <ws2tcpip.h>

// Windows headers
#include <windows.h>

// OpenGL headers
#include <gl/GL.h>

// raylib headers
#include "raylib.h"
#include "raygui.h"

// Standard library
#include <iostream>
#include <vector>
#include <string>
```

**Code Formatting**:
- **Indentation**: 4 spaces (no tabs)
- **Braces**: Opening brace on same line
- **Line length**: Max 120 characters
- **Comments**: Use `//` for single line, `/* */` for blocks

```cpp
// Good
if (condition) {
    doSomething();
} else {
    doSomethingElse();
}

// Bad
if(condition)
{
    doSomething();
}
else
{
    doSomethingElse();
}
```

### Best Practices

**Memory Management**:
```cpp
// Prefer smart pointers
std::unique_ptr<Player> player = std::make_unique<Player>();

// Use RAII for resources
class TextureManager {
    ~TextureManager() {
        UnloadTexture(m_texture);  // Auto cleanup
    }
};
```

**Error Handling**:
```cpp
// Check return values
if (!LoadModel("player.glb")) {
    std::cerr << "[ERROR] Failed to load model" << std::endl;
    // Fallback or exit gracefully
}

// Log errors clearly
std::cerr << "[" << __FUNCTION__ << "] Error: " << message << std::endl;
```

**Network Protocol Changes**:
```cpp
// ALWAYS update both client and server when changing packets!
// 1. Add to NetworkProtocol.h
// 2. Update client packet handler
// 3. Update server packet handler
// 4. Test both sides!

// Example: Adding new packet
enum PacketType : uint16_t {
    // ... existing packets
    PACKET_NEW_FEATURE = 350,  // Add in correct range
};
```

---

## Making Changes

### Branching Strategy

```bash
# Create a feature branch
git checkout -b feature/your-feature-name

# Or for bug fixes
git checkout -b fix/bug-description
```

### Development Workflow

1. **Make Changes** - Edit code in your branch
2. **Build** - Ensure project compiles
3. **Test** - Verify functionality works
4. **Commit** - Write clear commit messages
5. **Push** - Push to your branch
6. **Pull Request** - Submit for review

### Commit Message Format

```
<type>: <short summary> (max 50 chars)

<detailed description>
- What changed
- Why it changed
- Any breaking changes

Fixes #123 (if applicable)
```

**Types**: `feat`, `fix`, `docs`, `refactor`, `test`, `chore`

**Examples**:
```
feat: Add weapon attachment system

- Added attachment slots to weapon items
- Implemented stat modifiers for attachments
- Updated UI to show equipped attachments

Closes #45
```

```
fix: Resolve corpse looting distance check

Server was using squared distance instead of actual distance.
Changed proximity check to use proper distance calculation.

Fixes #67
```

---

## Testing

### Manual Testing

**For Client Changes**:
1. Build client in Debug mode
2. Run server: `x64\Debug\ExtractionShooterServer.exe`
3. Run client: `x64\Debug\ExtractionShooterRaylib.exe`
4. Test your changes in-game
5. Check for crashes, visual glitches, or errors

**For Server Changes**:
1. Build server in Debug mode
2. Run server with console visible
3. Run multiple clients to test multiplayer
4. Monitor server console for errors
5. Check server.log for warnings

**For Network Changes**:
1. Test with 2+ clients simultaneously
2. Test high-latency scenarios (if possible)
3. Verify packet handling on both sides
4. Check for memory leaks (use Task Manager)

### Automated Testing (Future)

We're working on adding:
- Unit tests for core systems
- Integration tests for networking
- Performance benchmarks

---

## Submitting Changes

### Pre-Submission Checklist

Before submitting a pull request:

- [ ] Code compiles without warnings (Release mode)
- [ ] Tested functionality manually
- [ ] No console errors or crashes
- [ ] Updated documentation if needed
- [ ] Followed coding standards
- [ ] Commit messages are clear
- [ ] No unnecessary files committed (build artifacts, etc.)

### Pull Request Template

```markdown
## Description
Brief description of changes

## Type of Change
- [ ] Bug fix
- [ ] New feature
- [ ] Breaking change
- [ ] Documentation update

## Testing Done
- Describe testing performed
- Include screenshots/videos if applicable

## Checklist
- [ ] Code compiles
- [ ] Tested in-game
- [ ] Documentation updated
- [ ] No merge conflicts
```

---

## Common Tasks

### Adding a New Item

**1. Define in ItemDatabase.h**:
```cpp
// In ItemDatabase constructor
items[ITEM_NEW_WEAPON] = Item(
    ITEM_NEW_WEAPON,
    "M16A4",
    ItemType::WEAPON,
    ItemRarity::EPIC,
    45000,      // Base value
    1,          // Weight
    1,          // Stack size
    60,         // Damage
    800,        // Fire rate
    true        // Full auto
);
```

**2. Update ItemID enum**:
```cpp
enum ItemID {
    // ... existing items
    ITEM_NEW_WEAPON = 51,
};
```

**3. Add to merchant**:
```cpp
// In MerchantManager.cpp
prapor.addItem(ITEM_NEW_WEAPON, 10);  // Stock quantity
```

**4. Test**: Buy, equip, use in raid

---

### Adding a New Animation

**1. Export animation from Blender/Mixamo**:
- Export as glTF 2.0 (.glb)
- Ensure skeleton matches player model
- Note frame range

**2. Update AnimationType enum**:
```cpp
enum class AnimationType {
    // ... existing
    NEW_ANIMATION
};
```

**3. Add animation clip**:
```cpp
// In AnimationController::setupDefaultClips()
m_clips[AnimationType::NEW_ANIMATION] = AnimationClip(
    "NewAnim",   // Name
    0,           // Start frame
    30,          // End frame
    1.0f,        // Speed
    false        // Loop?
);
```

**4. Trigger in code**:
```cpp
animController.play(AnimationType::NEW_ANIMATION);
```

---

### Adding a New Packet Type

**1. Define in NetworkProtocol.h**:
```cpp
enum PacketType : uint16_t {
    // ... existing packets
    PACKET_MY_NEW_FEATURE = 399,  // Use next available ID
};

// Define packet structure
struct PacketMyNewFeature {
    uint32_t data1;
    float data2;
    char message[64];
};
```

**2. Server-side handler** (in ServerMain.cpp or manager):
```cpp
case PACKET_MY_NEW_FEATURE: {
    PacketMyNewFeature* pkt = (PacketMyNewFeature*)payload;
    handleMyNewFeature(clientId, pkt);
    break;
}
```

**3. Client-side handler** (in RaylibGameClient or NetworkClient):
```cpp
case PACKET_MY_NEW_FEATURE: {
    PacketMyNewFeature* pkt = (PacketMyNewFeature*)payload;
    processMyNewFeature(pkt);
    break;
}
```

**4. Send packet**:
```cpp
// Client sends to server
PacketMyNewFeature pkt;
pkt.data1 = 42;
pkt.data2 = 3.14f;
strcpy_s(pkt.message, "Hello");
networkClient->sendPacket(PACKET_MY_NEW_FEATURE, &pkt, sizeof(pkt));
```

**CRITICAL**: Test both sending and receiving!

---

### Adding a New Sound Effect

**1. Prepare audio file**:
- Format: WAV (16-bit, 44.1kHz recommended)
- Length: 0.1-1.0 seconds for SFX
- Place in `resources/sounds/my_sound.wav`

**2. Add to SoundType enum**:
```cpp
enum class SoundType {
    // ... existing
    MY_NEW_SOUND
};
```

**3. Load sound**:
```cpp
// In SoundSystem::loadSounds()
m_sounds[SoundType::MY_NEW_SOUND] = LoadSound("resources/sounds/my_sound.wav");
```

**4. Play sound**:
```cpp
// 2D sound
soundSystem.play(SoundType::MY_NEW_SOUND, 0.8f);  // 80% volume

// 3D sound
soundSystem.play3D(SoundType::MY_NEW_SOUND, playerPos, 50.0f);  // 50m range
```

---

### Modifying Server Configuration

**1. Add to server_config.ini**:
```ini
[MyNewSection]
MyNewSetting=100
```

**2. Load in ServerMain.cpp** or relevant manager:
```cpp
// Add member variable
int m_myNewSetting;

// In loadConfig()
m_myNewSetting = GetPrivateProfileIntA(
    "MyNewSection",
    "MyNewSetting",
    100,  // Default value
    "server_config.ini"
);
```

**3. Use the setting**:
```cpp
if (value > m_myNewSetting) {
    // Do something
}
```

---

## Debugging Tips

### Visual Studio Debugging

**Breakpoints**:
- F9: Toggle breakpoint
- F5: Start debugging
- F10: Step over
- F11: Step into

**Watch Variables**:
- Right-click variable → Add Watch
- Monitor values in Watch window

**Debug Output**:
```cpp
// Use std::cerr for debug output (shown in console)
std::cerr << "[DEBUG] Player position: " << x << ", " << y << std::endl;
```

### Network Debugging

**Server Console**:
- Watch for `[ERROR]`, `[WARNING]` messages
- Monitor packet types being received
- Check connection status

**Wireshark** (advanced):
- Capture packets on port 7777 (TCP)
- Verify packet structure matches protocol
- Check for packet loss or corruption

### Common Issues

**"Unresolved external symbol"**:
- Missing .cpp file in project
- Function declared but not defined
- Linker issue - check Additional Dependencies

**"Access violation"**:
- Null pointer dereference
- Out-of-bounds array access
- Use debugger to find exact line

**"LNK2005: already defined"**:
- Header included multiple times
- Missing include guards
- Move implementation to .cpp file

---

## Resources

### Documentation
- [raylib Cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html)
- [raygui Documentation](https://github.com/raysan5/raygui)
- [Winsock2 Reference](https://docs.microsoft.com/en-us/windows/win32/winsock/windows-sockets-start-page-2)

### Learning Resources
- [C++ Reference](https://en.cppreference.com/)
- [Game Networking](https://gafferongames.com/)
- [3D Math Primer](https://gamemath.com/)

### Project Documentation
- [RAYLIB_README.md](RAYLIB_README.md) - Technical architecture
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Common issues
- [DOCUMENTATION_INDEX.md](DOCUMENTATION_INDEX.md) - All docs

---

## Getting Help

**Before asking**:
1. Check existing documentation
2. Search closed issues
3. Review troubleshooting guide

**When asking for help**:
- Describe what you're trying to do
- Show what you've tried
- Include error messages (full text)
- Provide code snippets
- Mention your environment (OS, VS version, etc.)

**Where to ask**:
- GitHub Issues - for bugs and feature requests
- Discussions - for general questions
- Pull Requests - for code review

---

## Code of Conduct

- Be respectful and constructive
- Focus on the code, not the person
- Welcome newcomers
- Credit others' work
- Keep discussions on-topic

---

## License

By contributing, you agree that your contributions will be licensed under the same license as the project.

---

**Thank you for contributing to Extraction Shooter!** 🎮

Your efforts help make this project better for everyone.
