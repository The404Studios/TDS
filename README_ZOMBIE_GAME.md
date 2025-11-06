# 🧟 First-Person Zombie Survival Game

A complete first-person zombie survival shooter built with OpenGL featuring advanced AI, base building, and wave-based gameplay.

## 🎮 Features

### Core Gameplay
- **First-Person Shooter Controls**: Full WASD movement with mouse look
- **Multiple Weapons**: Pistol, Shotgun, Rifle, Machine Gun, and Sniper Rifle
- **Zombie AI**: Advanced pathfinding using NavMesh A* algorithm
- **Wave-Based Survival**: Increasing difficulty with more zombies each wave
- **Base Building**: Construct defensive structures to survive longer
- **Resource Management**: Earn money from kills to build and upgrade

### Graphics & Rendering
- **3D Weapon Models**: Procedurally generated guns with realistic proportions
- **Animated Zombies**: Full body animation with walking cycles
- **Advanced Lighting System**:
  - Directional light (sun)
  - Dynamic point light (player flashlight)
  - Ambient lighting
  - Fog effects for atmosphere
- **Particle Systems**:
  - Blood splatters
  - Muzzle flashes
  - Impact effects
- **Proper Vertex Rendering**: All models use OpenGL vertex arrays with normals

### Building System
1. **Walls** - Basic defensive barriers ($100)
2. **Turrets** - Automated defense turrets ($250)
3. **Ammo Stations** - Resupply ammunition ($150)
4. **Health Stations** - Restore health ($150)
5. **Barricades** - Wooden fortifications ($75)

All buildings can be upgraded for increased effectiveness!

### UI Features
- Health bar with visual feedback
- Ammo counter with reload indicator
- Wave information display
- Money and kill counter
- FPS display
- Build mode overlay
- Crosshair with center dot

## 🎯 Controls

### Movement
- **W/A/S/D** - Move forward/left/backward/right
- **Mouse** - Look around
- **Shift** - Sprint (1.5x speed)

### Combat
- **Left Mouse Button** - Shoot
- **R** - Reload weapon
- **1-2** - Switch weapons (more can be added)

### Building
- **B** - Toggle build mode
- **1-5** (in build mode) - Select building type
- **Left Click** (in build mode) - Place building

### Game
- **N** - Start next wave
- **ESC** - Exit game

## 🛠️ Building the Game

### Requirements
- Windows 10/11
- Visual Studio 2022 with C++ toolset
- OpenGL support (built into Windows)

### Compilation
1. Open `NeuralCivilization.vcxproj` in Visual Studio 2022
2. Select configuration (Debug or Release)
3. Build Solution (F7)
4. The game executable will be in the output directory

### Configuration Options
- **Platform**: Win32 or x64
- **Configuration**: Debug (for development) or Release (for performance)

## 🎲 Gameplay Tips

1. **Early Waves**: Use the pistol to conserve ammo and build up money
2. **Build Smart**: Place turrets in strategic locations to cover multiple angles
3. **Stay Mobile**: Don't get surrounded by zombies
4. **Manage Ammo**: Reload during safe moments, not during combat
5. **Upgrade**: Save money to upgrade turrets for better defense
6. **Use Walls**: Create choke points to funnel zombies

## 📊 Game Mechanics

### Weapons

| Weapon | Damage | Fire Rate | Range | Accuracy | Clip Size | Max Ammo |
|--------|--------|-----------|-------|----------|-----------|----------|
| Pistol | 25 | 0.3s | 50m | 95% | 12 | 120 |
| Shotgun | 60 | 1.0s | 20m | 70% | 6 | 48 |
| Rifle | 40 | 0.15s | 80m | 90% | 30 | 180 |
| Machine Gun | 20 | 0.08s | 60m | 75% | 100 | 400 |
| Sniper | 150 | 1.5s | 150m | 98% | 5 | 30 |

### Zombie Stats (Scale with Waves)
- **Health**: 100 + (Wave × 20)
- **Speed**: 8.0 + (Wave × 0.5)
- **Damage**: 10 + (Wave × 2)
- **Count per Wave**: 5 + (Wave × 5)

### Player Stats
- **Health**: 100 HP
- **Speed**: 15 units/second (22.5 sprinting)
- **Starting Money**: $0
- **Money per Kill**: $10 × Wave Number

## 🌐 NavMesh System

The game features a sophisticated navigation mesh for zombie pathfinding:

- **Grid-based NavMesh**: 20×20 grid across 200×200 world
- **A* Pathfinding**: Optimal path calculation
- **8-directional movement**: Zombies can move diagonally
- **Dynamic obstacle avoidance**: Zombies navigate around buildings
- **Walkability checks**: Only valid terrain is used for navigation

## 🎨 Technical Details

### Rendering Pipeline
1. **3D World Rendering**
   - Terrain with grid overlay
   - Building models with health-based coloring
   - Animated zombie models
   - Bullet tracers

2. **Lighting System**
   - Multiple light sources (GL_LIGHT0, GL_LIGHT1)
   - Smooth shading with proper normals
   - Spotlight for player flashlight
   - Fog rendering for depth

3. **2D UI Overlay**
   - Orthographic projection for UI elements
   - Text rendering using Windows fonts
   - Health/ammo bars with visual feedback
   - Crosshair rendering

### Performance Features
- **Particle pooling**: Efficient particle management
- **View frustum consideration**: All objects rendered within view
- **Fixed timestep updates**: Consistent game logic
- **FPS cap**: ~120 FPS for smooth gameplay
- **Dead zombie cleanup**: Automatic removal after 5 seconds

## 📝 Code Structure

```
ZombieGame.h           - Game class definitions
ZombieGame.cpp         - Game logic implementation
ZombieGameMain.cpp     - Rendering and main game loop
NeuralCivilization.vcxproj - Visual Studio project file
```

### Key Classes
- **Player**: First-person controller with weapons
- **Zombie**: AI-controlled enemy with pathfinding
- **Weapon**: Weapon system with different types
- **BaseBuilding**: Defensive structures with upgrades
- **GameWorld**: Main game manager
- **NavMeshNode**: Navigation graph for pathfinding

## 🐛 Known Limitations

- Terrain is currently flat (heightmap system in place for future use)
- Maximum of 100 waves implemented
- Buildings cannot be destroyed by player (only by zombies)
- No save/load system (continuous gameplay)

## 🚀 Future Enhancements

- [ ] Terrain height variation using the heightmap system
- [ ] More weapon types (melee, explosives)
- [ ] Zombie variety (fast, tank, boss types)
- [ ] Power-ups and special abilities
- [ ] Multiplayer support
- [ ] Save game functionality
- [ ] Sound effects and music
- [ ] More building types and traps

## 📜 License

This is a demonstration project created for educational purposes.

## 🎉 Credits

Built with:
- **OpenGL** for 3D rendering
- **Windows API** for window management
- **Custom A* implementation** for pathfinding
- **Procedural modeling** for all 3D assets

---

**Enjoy surviving the zombie apocalypse!** 🧟‍♂️🔫
