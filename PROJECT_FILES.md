# Visual Studio Project Files

This document explains the Visual Studio project structure for the Extraction Shooter multiplayer game.

## Project Files

### ExtractionShooterClient.vcxproj
**Location:** Root directory
**Purpose:** Client application (multiplayer game client)
**Entry Point:** `ClientMain.cpp`

**Includes:**
- ClientMain.cpp (entry point with WinMain)
- Common/ headers (NetworkProtocol, DataStructures, ItemDatabase)
- Client/ headers (NetworkClient, UIManager, LoginUI, LobbyUI, MainMenuUI, GameClient)

**Dependencies:**
- ws2_32.lib (Winsock2 networking)
- opengl32.lib (OpenGL rendering)
- glu32.lib (OpenGL utilities)
- user32.lib (Windows API)
- gdi32.lib (GDI for window management)

**Build Output:** ExtractionShooterClient.exe (Windows GUI application)

### ExtractionShooterServer.vcxproj
**Location:** Root directory
**Purpose:** Dedicated server (authoritative game server)
**Entry Point:** `Server/ServerMain.cpp`

**Includes:**
- Server/ServerMain.cpp (entry point with main())
- Common/ headers (NetworkProtocol, DataStructures, ItemDatabase)
- Server/ headers (NetworkServer, AuthManager, LobbyManager, FriendManager, MatchManager, MerchantManager, PersistenceManager)

**Dependencies:**
- ws2_32.lib (Winsock2 networking)

**Build Output:** ExtractionShooterServer.exe (Console application)

## Configuration

Both projects are configured for:
- **C++ Standard:** C++17
- **Platform Toolset:** v143 (Visual Studio 2022)
- **Character Set:** Unicode
- **Windows SDK:** 10.0

**Configurations:**
- Debug|Win32
- Release|Win32
- Debug|x64
- Release|x64

## Include Directories

Both projects have these include directories configured:
- `$(ProjectDir)` - Root directory
- `$(ProjectDir)Common` - Shared networking and data structures
- `$(ProjectDir)Client` or `$(ProjectDir)Server` - Client or server specific

This allows headers to be included with relative paths like:
```cpp
#include "Common/NetworkProtocol.h"
#include "Client/NetworkClient.h"
#include "Server/AuthManager.h"
```

## How to Build

### Using Visual Studio

1. Open `ExtractionShooterServer.vcxproj` OR `ExtractionShooterClient.vcxproj`
2. Select configuration (Debug/Release) and platform (Win32/x64)
3. Build → Build Solution (Ctrl+Shift+B)
4. Run with F5 (Debug) or Ctrl+F5 (Run without debugging)

### Using MSBuild (Command Line)

**Server:**
```cmd
msbuild ExtractionShooterServer.vcxproj /p:Configuration=Release /p:Platform=x64
```

**Client:**
```cmd
msbuild ExtractionShooterClient.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Using CL.exe (Command Line)

**Server:**
```cmd
cl /EHsc /std:c++17 /I. /ICommon /IServer Server\ServerMain.cpp /link ws2_32.lib /OUT:Server.exe
```

**Client:**
```cmd
cl /EHsc /std:c++17 /I. /ICommon /IClient ClientMain.cpp /link ws2_32.lib opengl32.lib glu32.lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /OUT:Client.exe
```

## Removed Old Files

The following old project files have been **archived** to `Archive/SinglePlayer/`:
- `NeuralCivilization.vcxproj` - Old civilization game client
- `Server/NeuralCivilizationServer.vcxproj` - Old civilization game server

These files are no longer relevant to the multiplayer extraction shooter and were causing confusion.

## File Structure

```
TDS/
├── ExtractionShooterClient.vcxproj    ← NEW CLIENT PROJECT
├── ExtractionShooterServer.vcxproj    ← NEW SERVER PROJECT
├── ClientMain.cpp                     ← Client entry point
├── Common/                            ← Shared code
│   ├── NetworkProtocol.h
│   ├── DataStructures.h
│   └── ItemDatabase.h
├── Client/                            ← Client-specific
│   ├── NetworkClient.h
│   ├── UIManager.h
│   ├── LoginUI.h
│   ├── LobbyUI.h
│   ├── MainMenuUI.h
│   └── GameClient.h
├── Server/                            ← Server-specific
│   ├── ServerMain.cpp                 ← Server entry point
│   ├── NetworkServer.h
│   ├── AuthManager.h
│   ├── LobbyManager.h
│   ├── FriendManager.h
│   ├── MatchManager.h
│   ├── MerchantManager.h
│   └── PersistenceManager.h
└── Archive/SinglePlayer/              ← Old code
    ├── NeuralCivilization.vcxproj     (archived)
    └── NeuralCivilizationServer.vcxproj (archived)
```

## What Changed

### Old Structure (Removed)
- `NeuralCivilization.vcxproj` referenced:
  - main.cpp (deleted - old civilization game)
  - CivilizationAI.cpp/h
  - NetworkManager.cpp/h
  - GameplayFeatures.cpp/h
  - PlayerCharacter.h (archived)
  - ExtractionShooter.h (archived)

### New Structure (Current)
- `ExtractionShooterClient.vcxproj` references:
  - ClientMain.cpp (NEW - multiplayer client)
  - Common/ shared headers
  - Client/ UI and networking headers

- `ExtractionShooterServer.vcxproj` references:
  - Server/ServerMain.cpp (NEW - dedicated server)
  - Common/ shared headers
  - Server/ manager headers

## Troubleshooting

### "Cannot find ClientMain.cpp"
- Ensure you're opening `ExtractionShooterClient.vcxproj` from the root directory
- Check that `ClientMain.cpp` exists in the root directory

### "Cannot find Server/ServerMain.cpp"
- Ensure you're opening `ExtractionShooterServer.vcxproj` from the root directory
- Check that `Server/ServerMain.cpp` exists

### "Unresolved external symbols"
- Make sure all required libraries are linked (ws2_32.lib, opengl32.lib, etc.)
- Check that the correct subsystem is set (Windows for client, Console for server)

### "Cannot open include file"
- Verify include directories are set correctly in project properties
- Check that Common/, Client/, and Server/ directories exist

---

**Project files completely rewritten for multiplayer extraction shooter - No more civilization game references!**
