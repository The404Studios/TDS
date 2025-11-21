# Documentation Index

Complete guide to all documentation for the Extraction Shooter project.

---

## Getting Started

### 🚀 [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md)
**Start here!** Get the game running in 5 minutes.
- Download dependencies
- Build the project
- Run your first game
- Add custom models (optional)
- Free asset resources

**For**: New users, first-time setup

---

### 🔨 [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)
Detailed build process and project structure.
- Dependency download requirements
- Visual Studio build steps
- Command-line build commands
- Project overview
- Quick troubleshooting

**For**: Setting up development environment

---

### ⚠️ [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
Comprehensive solutions to 20+ common issues.
- Build and compilation errors
- Runtime errors and crashes
- Networking and connectivity problems
- Data/account issues
- Performance optimization
- Platform-specific issues
- Quick diagnostic commands

**For**: When something doesn't work

---

## Gameplay & Controls

### 🎮 [CONTROLS.md](CONTROLS.md)
Complete control reference for players.
- Movement, combat, and interaction controls
- UI navigation and hotkeys
- HUD elements explained
- Control customization guide
- Tips and tricks for gameplay
- Quick reference control card

**For**: Players learning the game, creating control overlays

---

## Development & Testing

### 🤝 [CONTRIBUTING.md](CONTRIBUTING.md)
Developer's guide to contributing code.
- Development setup and workflow
- Code structure and standards
- Making changes and testing
- Common development tasks
- Debugging tips
- Pull request guidelines

**For**: Developers wanting to modify or contribute

---

### 🎮 [MULTIPLAYER_TESTING_GUIDE.md](MULTIPLAYER_TESTING_GUIDE.md)
How to test multiplayer features.
- 9 detailed test scenarios
- Expected server console output
- Performance benchmarks
- Common multiplayer issues
- Network stress testing
- Automated testing (future)

**For**: Testing multiplayer gameplay

---

### 📘 [RAYLIB_README.md](RAYLIB_README.md)
Technical documentation for the raylib client.
- Architecture overview
- Animation system details
- Networking protocol
- Audio system
- Particle effects
- Code structure
- API reference

**For**: Understanding the codebase, technical reference

---

## Deployment & Production

### 🌐 [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)
Complete production deployment manual.
- Local deployment
- LAN deployment (home/office)
- Public server setup
- Cloud hosting (AWS, DigitalOcean, etc.)
- Security hardening
- Performance optimization
- Monitoring and maintenance
- Backup and recovery
- Scaling strategies
- Cost estimates

**For**: Deploying to production, hosting servers

---

## Configuration & Tools

### ⚙️ [server_config.ini](server_config.ini)
Server configuration file.
- Network settings
- Match and raid settings
- Loot and AI configuration
- Anti-cheat settings
- Performance tuning
- Debug options

**For**: Customizing server behavior

---

### 🛠️ Development Tools

**verify_build_env.bat**
- Checks if build environment is ready
- Verifies dependencies are downloaded
- Validates MSBuild installation

**setup_and_run.bat**
- One-click setup and launch
- Downloads dependencies
- Builds all projects
- Runs the game

**run_game.bat**
- Quick launcher for built game
- Supports multiple clients (1-8)
- Automated server startup

**dev_tools.bat**
- Interactive developer utilities
- Build, clean, run options
- View logs, clear data
- Test account generation

**download_dependencies.bat**
- Downloads raylib 5.0
- Downloads raygui
- Downloads ENet (NAT)
- Creates resource directories

---

## Quick Reference by Task

### "I want to..."

#### ...get started quickly
→ [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md)

#### ...build the project
1. Run `download_dependencies.bat`
2. Run `verify_build_env.bat` (optional)
3. See [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)

#### ...fix a build error
1. Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Build Errors section
2. Run `verify_build_env.bat`
3. See [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md) - Troubleshooting

#### ...test multiplayer
→ [MULTIPLAYER_TESTING_GUIDE.md](MULTIPLAYER_TESTING_GUIDE.md)

#### ...deploy a server
1. [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) - Choose deployment type
2. Configure [server_config.ini](server_config.ini)
3. Follow security checklist in deployment guide

#### ...understand the code
→ [RAYLIB_README.md](RAYLIB_README.md) - Architecture section

#### ...add custom models
→ [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md) - Adding Custom 3D Models

#### ...configure the server
→ [server_config.ini](server_config.ini) with comments explaining each option

#### ...fix a networking issue
→ [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Networking Issues section

#### ...optimize performance
1. [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) - Performance Optimization
2. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Low FPS section
3. [server_config.ini](server_config.ini) - Performance section

#### ...learn the controls
→ [CONTROLS.md](CONTROLS.md) - Complete control reference

#### ...contribute code
→ [CONTRIBUTING.md](CONTRIBUTING.md) - Developer contribution guide

---

## Documentation by Audience

### 🆕 New Users (Never used this project)
1. [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md)
2. Run `setup_and_run.bat`
3. [CONTROLS.md](CONTROLS.md) - Learn the controls
4. If issues: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

### 🎮 Players (Want to play the game)
1. [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md) - Setup
2. [CONTROLS.md](CONTROLS.md) - Learn controls
3. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Fix issues

### 💻 Developers (Want to modify code)
1. [BUILD_INSTRUCTIONS.md](BUILD_INSTRUCTIONS.md)
2. [CONTRIBUTING.md](CONTRIBUTING.md)
3. [RAYLIB_README.md](RAYLIB_README.md)
4. [MULTIPLAYER_TESTING_GUIDE.md](MULTIPLAYER_TESTING_GUIDE.md)
4. Use `dev_tools.bat` for development workflow

### 🖥️ Server Administrators (Hosting servers)
1. [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md)
2. [server_config.ini](server_config.ini)
3. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Networking section
4. [MULTIPLAYER_TESTING_GUIDE.md](MULTIPLAYER_TESTING_GUIDE.md) - Performance benchmarks

### 🎨 Content Creators (Adding models/sounds)
1. [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md) - Custom models/sounds sections
2. [RAYLIB_README.md](RAYLIB_README.md) - Animation system
3. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - Asset loading issues

### 🧪 Testers (Quality assurance)
1. [MULTIPLAYER_TESTING_GUIDE.md](MULTIPLAYER_TESTING_GUIDE.md)
2. Use `run_game.bat` to launch multiple clients
3. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) - For reporting issues

---

## File Structure Overview

```
TDS/
├── *.md                           # Documentation (root directory)
│   ├── README.md                  # Project overview
│   ├── QUICK_START_GUIDE.md       # ⭐ Start here!
│   ├── BUILD_INSTRUCTIONS.md      # Build process
│   ├── TROUBLESHOOTING.md         # Problem solutions
│   ├── CONTROLS.md                # Game controls reference
│   ├── CONTRIBUTING.md            # Developer guide
│   ├── MULTIPLAYER_TESTING_GUIDE.md  # Testing guide
│   ├── DEPLOYMENT_GUIDE.md        # Production deployment
│   ├── RAYLIB_README.md           # Technical docs
│   └── DOCUMENTATION_INDEX.md     # This file
│
├── src/                           # Source code
│   ├── client/                    # Raylib game client
│   │   ├── RaylibGameClient.h     # Main client
│   │   ├── RaylibMain.cpp         # Entry point
│   │   ├── animation/             # Animation system
│   │   ├── audio/                 # Sound system
│   │   └── effects/               # Particle effects
│   ├── server/                    # Game server
│   │   └── managers/              # Game logic
│   ├── common/                    # Shared code
│   │   └── CorpseSystem.h         # Body looting
│   └── natpunch/                  # NAT punchthrough
│
├── dependencies/                  # External libraries (created by script)
│   ├── raylib/                    # raylib 5.0
│   ├── raygui/                    # raygui UI
│   └── enet/                      # NAT traversal
│
├── resources/                     # Game assets (optional)
│   ├── models/                    # 3D models (.glb)
│   ├── sounds/                    # Audio (.wav)
│   └── textures/                  # Textures (.png)
│
├── Data/                          # Server data (created at runtime)
│   └── *.dat                      # Player accounts
│
├── x64/Release/                   # Build output
│   ├── ExtractionShooterServer.exe   # Game server
│   ├── ExtractionShooterRaylib.exe   # Game client
│   └── NatPunchServer.exe            # NAT server
│
├── *.bat                          # Utility scripts
│   ├── download_dependencies.bat  # Download raylib/raygui
│   ├── verify_build_env.bat       # Check setup
│   ├── setup_and_run.bat          # One-click setup
│   ├── run_game.bat               # Quick launcher
│   └── dev_tools.bat              # Developer menu
│
├── server_config.ini              # Server configuration
├── *.vcxproj                      # Visual Studio projects
└── ExtractionShooter.sln          # Visual Studio solution
```

---

## Common Workflows

### First Time Setup
```cmd
# 1. Download dependencies
download_dependencies.bat

# 2. Verify environment
verify_build_env.bat

# 3. Build and run
setup_and_run.bat
```

### Daily Development
```cmd
# Use interactive menu
dev_tools.bat

# Or manual:
# Edit code in Visual Studio
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Debug /p:Platform=x64
x64\Debug\ExtractionShooterRaylib.exe
```

### Testing Multiplayer
```cmd
# Launch multiple clients
run_game.bat
# Enter: 3 (for 3 clients)

# Follow testing scenarios in MULTIPLAYER_TESTING_GUIDE.md
```

### Deploying to Production
```cmd
# 1. Build in Release mode
msbuild ExtractionShooter.sln /p:Configuration=Release /p:Platform=x64

# 2. Configure server
# Edit server_config.ini (disable debug, enable anti-cheat)

# 3. Upload to server
# See DEPLOYMENT_GUIDE.md for cloud hosting steps

# 4. Set up monitoring
# See DEPLOYMENT_GUIDE.md - Monitoring section
```

---

## Getting Help

### Step-by-step help process:

1. **Check relevant documentation** (use index above)
2. **Run verification**: `verify_build_env.bat`
3. **Check troubleshooting**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
4. **Search existing issues**: GitHub Issues page
5. **Report new issue**: Include:
   - Full error message
   - Steps to reproduce
   - System specs
   - Relevant log files

---

## Contributing

Want to improve the docs? Guidelines:

- **Clarity**: Write for beginners
- **Examples**: Show, don't just tell
- **Testing**: Verify steps work before documenting
- **Structure**: Follow existing formatting
- **Cross-reference**: Link to related docs

---

## Version Information

**Project**: Extraction Shooter - Multiplayer 3D Looter Shooter
**Engine**: raylib 5.0 + raygui
**Platform**: Windows (Visual Studio 2022)
**Language**: C++17

**Key Technologies**:
- raylib 5.0 - 3D graphics
- raygui - Immediate mode GUI
- ENet - NAT traversal
- Winsock2 - Networking
- File-based persistence

---

**Need help finding the right documentation? Check the "I want to..." section above or start with [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md)!**
