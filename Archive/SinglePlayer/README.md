# Single-Player Extraction Shooter Archive

This directory contains the **original single-player** implementation of the extraction shooter that was created before the multiplayer rewrite.

## Archived on: 2025-11-06

These files were archived because:
1. The project was completely rewritten for **multiplayer functionality**
2. These files were causing compilation conflicts with the new codebase
3. They are preserved here for reference and historical purposes

## Archived Files

### Code Files
- **ExtractionShooter.h** (1,400+ lines) - Original single-player FPS game mode
  - FPS gameplay with AI enemies
  - Weapon firing with raycast hit detection
  - Loot spawns and corpse looting
  - Extraction zones
  - Combat system with damage and reload

- **PlayerCharacter.h** - Original player character class
- **PlayerData.h** (343 lines) - Original persistent stash system
- **InventorySystem.h** (520 lines) - Original grid-based inventory

- **main_old.cpp** (1,100+ lines) - Original single-player entry point

### Documentation Files
- **COMBAT_CHANGELOG.md** - Technical documentation of combat system
- **EXTRACTION_SHOOTER_README.md** - User-facing documentation
- **FPS_README.md** - FPS gameplay documentation

## What Replaced This

The new multiplayer implementation can be found in:
- **Common/** - Shared networking and data structures
- **Server/** - Authoritative server with all game logic
- **Client/** - Multiplayer client with UI and networking
- **ClientMain.cpp** - New client entry point
- **Server/ServerMain.cpp** - New server entry point

See **MULTIPLAYER_IMPLEMENTATION.md** in the root directory for details.

## Key Differences

**Old Single-Player:**
- Local FPS gameplay
- AI enemies (local)
- Local loot spawns
- No networking
- No authentication
- No lobbies
- No party system
- Standalone executable

**New Multiplayer:**
- Client-server architecture
- Authoritative server
- TCP networking
- Login/registration system
- Lobby and party system
- Matchmaking queue
- Multiple players per raid
- Friend system
- Merchant system
- Persistent account data

## Can I Still Use The Old Code?

Yes! These files are fully functional as single-player code. To use them:

1. Copy files from Archive/SinglePlayer/ back to root
2. Compile with: `cl /EHsc main_old.cpp /link opengl32.lib glu32.lib user32.lib gdi32.lib`
3. Run the executable

**Note:** The old code will conflict with the new multiplayer code if both are present in the root directory.

---

**Archived for historical reference - The multiplayer version is now the primary codebase.**
