# 🎮 Neural Civilization - FPS Mode

## NEW FIRST-PERSON SHOOTER FEATURES

The game has been transformed into a **hybrid FPS/Strategy** experience where you play as a stick figure character in a living world populated by AI civilizations!

---

## 🎯 FPS Features

### Stick Figure Player
- **Procedurally animated stick figure** with inverse kinematics (IK)
- **Realistic leg movement** - legs adapt to terrain using IK solver
- **Walking animations** - natural step cycle with foot planting
- **Body physics** - head bob, arm sway, recoil

### Terrain System
- **Procedural terrain generation** using Perlin noise
- **Multiple biomes**: Grass, Dirt, Sand, Stone, Water, Snow
- **Heightmap-based** with smooth interpolation
- **Dynamic normals** for proper lighting
- **Collision detection** with terrain

### Weapon System
- **5 Weapon Types**:
  - **Pistol**: Balanced, good accuracy
  - **Rifle**: High damage, long range
  - **Shotgun**: Close range, spread damage
  - **SMG**: Fast fire rate, lower damage
  - **Sniper**: Very long range, high damage

- **Realistic mechanics**:
  - Magazine system with reloading
  - Recoil and accuracy spread
  - Bullet drop (future)
  - Different fire rates

### Main Menu
- **Start Game** - Begin FPS mode
- **Options** - Graphics/audio settings
- **Controls** - View key bindings
- **Exit** - Quit game

### In-Game HUD
- **Crosshair** - Adaptive aiming reticle
- **Health Bar** (top left)
- **Stamina Bar** (below health)
- **Ammo Counter** (bottom right)
- **Weapon Info** - Current weapon name
- **Minimap** - Top-down view with AI agents

### Pause Menu
- Press **ESC** to pause
- Resume, Options, Main Menu, Quit

---

## 🎮 FPS Controls

### Movement
- **W/A/S/D** - Move forward/left/backward/right
- **SPACE** - Jump
- **LEFT SHIFT** - Sprint (drains stamina)
- **LEFT CTRL** - Crouch
- **MOUSE** - Look around

### Combat
- **LEFT CLICK** - Shoot
- **RIGHT CLICK** - Aim down sights (future)
- **R** - Reload
- **SCROLL WHEEL** - Cycle weapons
- **1-5 KEYS** - Quick weapon select

### Game
- **ESC** - Pause menu
- **TAB** - Toggle AI civilization stats
- **M** - Toggle fullscreen minimap
- **F** - Toggle FPS counter

---

## 🤖 AI Civilizations Integration

The AI civilizations continue to evolve in the background while you play:

- **Watch them build** - AI agents construct buildings
- **Observe combat** - Factions wage war
- **Trade routes** - See diplomacy in action
- **Dynamic world** - World changes based on AI decisions

### Interaction Options (Future)
- Join a faction
- Trade with AI agents
- Attack enemy factions
- Defend allied settlements

---

## 🏗️ Technical Architecture

### New Files Added:

```
TDS/
├── TerrainManager.h/.cpp        - Procedural terrain generation
├── PlayerCharacter.h            - Stick figure FPS controller
├── GameUI.h                     - Main menu, HUD, pause menu
└── FPS_README.md                - This file
```

### Key Systems:

#### Inverse Kinematics (IK)
```cpp
class IKChain {
    // Solves leg positioning to reach foot targets
    // Ensures feet properly plant on terrain
    // Creates natural walking animation
};
```

#### Terrain Generation
```cpp
TerrainManager terrain(100, 100, 5.0f);
terrain.generate(12345); // Seed-based generation
float height = terrain.getHeight(x, z); // Query height
```

#### First-Person Camera
```cpp
// Camera follows player head position
Vector3 cameraPos = player.getCameraPosition();
Vector3 lookDir = player.getForwardVector();
gluLookAt(cameraPos.x, cameraPos.y, cameraPos.z,
          cameraPos.x + lookDir.x,
          cameraPos.y + lookDir.y,
          cameraPos.z + lookDir.z,
          0, 0, 1);
```

---

## 🎨 Visual Style

### Stick Figure Design
- **Minimalist aesthetic** - Clean lines
- **Procedural animation** - IK-driven movement
- **Readable silhouette** - Clear in any environment

### Color Scheme
- **Player**: White stick figure
- **Friendly AI**: Blue tint
- **Enemy AI**: Red tint
- **Neutral**: Gray tint

---

## 🚀 Performance

### Optimizations:
- **LOD system** for distant AI agents
- **Frustum culling** for terrain
- **Occlusion queries** for hidden objects
- **60 FPS target** on modern hardware

### System Requirements:
- **GPU**: OpenGL 2.0+
- **RAM**: 2GB+
- **CPU**: Dual-core 2.0GHz+

---

## 🎯 Gameplay Loop

1. **Spawn** in the world as a stick figure
2. **Explore** procedurally generated terrain
3. **Observe** AI civilizations building and fighting
4. **Collect** weapons and resources
5. **Engage** in combat with hostile AI
6. **Survive** as long as possible
7. **Watch** the world evolve around you

---

## 🔮 Future Features

### Planned Additions:
- [ ] **Weapon upgrades** - Attachments, skins
- [ ] **AI interaction** - Talk to/trade with AI agents
- [ ] **Faction joining** - Become part of a civilization
- [ ] **Base building** - Construct your own structures
- [ ] **Vehicles** - Cars, helicopters, boats
- [ ] **Multiplayer** - Co-op and PVP modes
- [ ] **Day/night cycle** - Dynamic lighting
- [ ] **Weather effects** - Rain, snow affecting gameplay
- [ ] **Crafting system** - Make items from resources
- [ ] **Skill tree** - Level up and unlock abilities

---

## 📖 How to Build

The FPS mode is integrated into the existing client:

```bash
# Open solution
Open NeuralCivilization.sln in Visual Studio

# Build client
Select "Debug x64" or "Release x64"
Build → Build NeuralCivilization

# Run
x64/Debug/NeuralCivilization.exe
```

---

## 🎮 Quick Start Guide

1. **Launch the game**
2. **Main Menu appears**
3. **Click "Start Game"**
4. **You spawn as a stick figure in the world**
5. **Use WASD to move, mouse to look**
6. **Left click to shoot**
7. **Explore and survive!**

---

## ⚙️ Configuration

### Camera Settings
```cpp
cameraFOV = 90.0f;         // Field of view
mouseSensitivity = 0.002f; // Look speed
```

### Player Settings
```cpp
moveSpeed = 15.0f;    // Walk speed
sprintSpeed = 25.0f;  // Sprint speed
jumpForce = 8.0f;     // Jump height
```

### Weapon Balance
```cpp
// Edit in PlayerCharacter.cpp
Weapon::Weapon(Type::RIFLE) {
    damage = 35.0f;
    fireRate = 6.0f; // Shots per second
    range = 500.0f;
    accuracy = 0.95f;
}
```

---

## 🐛 Known Issues

- Leg IK sometimes clips through steep terrain
- Weapon switching animation needs work
- AI agents don't react to player yet
- No save/load system

---

## 💡 Tips & Tricks

- **Sprint wisely** - Stamina regenerates slowly
- **High ground** - Better vantage point
- **Crouch** - Smaller target, more accurate
- **Reload in cover** - Don't get caught vulnerable
- **Watch AI** - Learn from their tactics
- **Use terrain** - Hills provide protection

---

**Enjoy the hybrid FPS/Strategy experience!** 🎯🤖

---

*Note: This is an early implementation. Many features are still in development. Check FPS_TODO.md for planned improvements.*
