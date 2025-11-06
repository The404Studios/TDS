# Extraction Shooter Mode

This game has been transformed into a **Tarkov-style extraction shooter** with persistent stash, inventory management, and raid mechanics!

## Overview

The game now features a complete extraction shooter experience where you:
1. Manage your **persistent stash** between raids
2. Equip gear and enter raids with your **loadout**
3. Explore a **procedurally generated 3D terrain**
4. Collect **loot** scattered around the map
5. Fight **AI enemies** (scavs)
6. Find **extraction zones** to escape with your loot
7. Keep your gear if you extract, or **lose it all if you die**

## Game Features

### 🎒 Persistent Stash System
- **10x30 grid stash** (standard edition size, Tarkov-style)
- Items persist between raids
- Grid-based inventory with different item sizes
- All items saved to `playerdata.txt`

### 📦 Inventory System
- **Grid-based inventory** like Tarkov
- Items have different sizes (1x1 to 4x5)
- Item categories:
  - Weapons (AK-74, M4A1, SVD, Glock, Kedr)
  - Armor (PACA, 6B3, Slick)
  - Helmets (SSh-68, ZSh, Altyn)
  - Backpacks (Scav, Berkut, Tri-Zip)
  - Medical items (AI-2, IFAK, Grizzly, Surv12)
  - Food & Water (Tushonka, MRE, Aquamari)
  - Valuables (Rolex, Bitcoin, LEDX, GPU)
  - Materials (Bolts, Wires, Gunpowder)
- Item rarities: Common, Uncommon, Rare, Epic, Legendary
- **Found in Raid (FiR)** status tracking

### 🎮 Raid Gameplay
- **First-person shooter** perspective
- WASD movement + mouse look
- Sprint with SPACE
- **30-minute raid timer** (extract before time runs out!)
- Procedurally generated terrain with biomes
- Dynamic loot spawns (50+ items per raid)
- AI enemy scavs (10 per raid)

### 🚁 Extraction Mechanics
- **3 extraction zones** per raid:
  - Railroad Bridge (8 second extract)
  - Scav Checkpoint (10 second extract)
  - Boat Dock (7 second extract)
- Stand in extraction zone and press **F** to extract
- Progress bar shows extraction countdown
- **Successfully extract** to keep all your loot!

### 💀 Death & Survival
- **Survival rate** tracking
- Stats: Raids completed, extractions, deaths, K/D
- **Lose all equipped gear on death** (no insurance system yet)
- Collected loot is lost if you die
- Persistent player level and experience

### 💰 Economy System
- Start with **500,000 roubles**
- Items have market values
- Track total gear value
- Save/load currency between sessions

## Controls

### Main Menu
- **1** - Open Stash
- **2** - Start Raid
- **ENTER** - Start raid from stash screen
- **ESC** - Quit game

### In-Raid Controls
- **WASD** - Move
- **Mouse** - Look around
- **SPACE** - Sprint
- **E** - Pick up loot
- **F** - Extract (when in extraction zone)
- **ESC** - Toggle inventory
- **Green zones** on terrain = extraction points!

### Post-Raid
- **ENTER** - Return to main menu

## Game Loop

```
1. Main Menu
   ↓
2. Stash Screen (manage items, equip gear)
   ↓
3. Enter Raid (FPS gameplay)
   ↓
4. Collect Loot + Fight Enemies
   ↓
5. Find Extraction Zone
   ↓
6. Extract Successfully OR Die
   ↓
7. Post-Raid Screen (transfer loot to stash)
   ↓
8. Back to Main Menu (loop)
```

## File Structure

### New Files Created
- **`InventorySystem.h`** - Complete inventory and item database
- **`PlayerData.h`** - Persistent player data, stash, and loadout
- **`ExtractionShooter.h`** - Main extraction shooter game mode
- **`EXTRACTION_SHOOTER_README.md`** - This file!

### Modified Files
- **`main.cpp`** - Integrated ExtractionShooter mode with input handling

### Existing FPS Components (Now Integrated!)
- **`PlayerCharacter.h`** - First-person stick figure player with IK
- **`TerrainManager.h/cpp`** - Procedural terrain generation (Perlin noise)
- **`GameUI.h`** - Menu and HUD systems

### Persistence
- **`playerdata.txt`** - Your profile (auto-saved after each raid)
  - Player name, level, XP, roubles
  - Stash contents
  - Current loadout
  - Statistics

## Item Database

### Weapons
| Item | Damage | Mag Size | Value | Rarity |
|------|--------|----------|-------|--------|
| AK-74 | 40 | 30 | 25,000₽ | Common |
| M4A1 | 45 | 30 | 35,000₽ | Uncommon |
| SVD | 85 | 10 | 55,000₽ | Rare |
| Glock 17 | 30 | 17 | 8,000₽ | Common |
| Kedr | 28 | 30 | 15,000₽ | Common |

### Armor
| Item | Class | Durability | Value | Rarity |
|------|-------|------------|-------|--------|
| PACA | 2 | 50 | 15,000₽ | Common |
| 6B3TM | 4 | 65 | 45,000₽ | Uncommon |
| Slick | 6 | 80 | 250,000₽ | Legendary |

### Valuables
| Item | Value | Rarity |
|------|-------|--------|
| Rolex | 65,000₽ | Rare |
| Bitcoin | 150,000₽ | Epic |
| LEDX | 450,000₽ | Legendary |
| GPU | 250,000₽ | Epic |

## Technical Implementation

### Rendering
- OpenGL 1.1 fixed-function pipeline
- First-person camera with yaw/pitch
- Procedural terrain rendering
- 3D loot visualization
- HUD overlay with health bars
- Extraction zone indicators

### Game States
1. **MAIN_MENU** - Start screen
2. **STASH_SCREEN** - Inventory management
3. **IN_RAID** - Active FPS gameplay
4. **EXTRACTING** - Countdown at extraction
5. **POST_RAID** - Loot summary screen
6. **DEATH_SCREEN** - Game over screen

### Persistence System
- Text-based save format
- Automatic save after extraction/death
- Inventory serialization
- Loadout tracking
- Statistics persistence

## Starting Gear

New players receive:
- 500,000 roubles
- AK-74 rifle
- Glock 17 pistol
- PACA armor
- SSh-68 helmet
- Scav backpack
- 2x 5.45x39 ammo
- 1x 9x18 ammo
- IFAK medical
- 2x AI-2 medkit
- Water + Tushonka

## Future Enhancements

Potential additions:
- [ ] Weapon firing and hit detection
- [ ] AI enemy combat behavior
- [ ] Insurance system
- [ ] Traders and market
- [ ] Weapon modding system
- [ ] More maps/raids
- [ ] Multiplayer raids
- [ ] Hideout upgrades
- [ ] Quest system
- [ ] Character skills
- [ ] Sound effects
- [ ] Advanced UI (drag & drop inventory)

## Building the Game

This is a **Windows-only** project using:
- Visual Studio 2019+
- OpenGL
- Windows API
- Winsock2 (for multiplayer)

Build with:
```bash
msbuild NeuralCivilization.sln /p:Configuration=Release /p:Platform=x64
```

Or open `NeuralCivilization.sln` in Visual Studio.

## Notes

- The old top-down strategy view still exists (set `g_useExtractionMode = false` in main.cpp)
- Player data saves automatically after each raid
- All loot is procedurally spawned each raid
- Extraction zones are fixed but can be randomized
- AI enemies are currently static (no pathfinding yet)
- Item pickup is proximity-based (2 meter radius)

---

**Enjoy your raids, PMC! Remember: Only what you extract survives!**
