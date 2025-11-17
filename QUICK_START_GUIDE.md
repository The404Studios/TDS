# Quick Start Guide - Extraction Shooter

Get up and running in 5 minutes!

## Step 1: Download Dependencies

Run the batch file to download raylib, raygui, and NAT punchthrough library:

```cmd
download_dependencies.bat
```

This will create:
- `dependencies/raylib/` - raylib 5.0
- `dependencies/raygui/` - raygui UI library
- `dependencies/enet/` - NAT punchthrough (optional)
- `resources/` folders for your assets

**Verify everything downloaded correctly** (optional but recommended):

```cmd
verify_build_env.bat
```

If it shows **[SUCCESS] Environment is ready to build!** you can proceed!

## Step 2: Build the Project

### Option A: Visual Studio (Recommended)

1. Open `ExtractionShooter.sln` in Visual Studio 2022
2. Select configuration: **Release** and platform: **x64**
3. Build → Build Solution (Ctrl+Shift+B)

### Option B: Command Line

```cmd
msbuild ExtractionShooter.sln /p:Configuration=Release /p:Platform=x64
```

## Step 3: Run the Game

### Start the Game Server

```cmd
cd x64\Release
ExtractionShooterServer.exe
```

Wait for: `[Server] Server started on port 7777`

### Start the Client

```cmd
ExtractionShooterRaylib.exe
```

The game will use placeholder models automatically if custom models aren't found.

## Step 4: Play!

1. **Login Screen**
   - Enter any username and password
   - Click "Register" or "Login"

2. **Main Menu**
   - Click "Enter Raid" to start playing

3. **In-Game**
   - **WASD** - Move around
   - **Shift** - Sprint
   - **Mouse** - Look around
   - **Left Click** - Shoot
   - **F** - Loot nearby corpses

4. **Multiplayer**
   - Run multiple clients to test multiplayer
   - All clients connect to localhost:7777

---

## Adding Custom 3D Models (Optional)

The game works perfectly with placeholder models, but you can add custom models:

### 1. Prepare Your Model

Export your 3D model as `.glb` (glTF 2.0) format from Blender, Maya, or your favorite 3D software.

**Recommended specs:**
- **Polycount**: 2,000-10,000 triangles
- **Textures**: 512x512 or 1024x1024
- **Animations**: 30 FPS, exported with model

### 2. Place Model Files

```
resources/
  models/
    player.glb           ← Player character model
    corpse.glb           ← Dead player model (optional)
    weapon.glb           ← Weapon model (optional)
  animations/
    player_anims.glb     ← Player animations (if separate)
  textures/
    ground.png           ← Ground texture
    player_diffuse.png   ← Player texture (if not embedded)
```

### 3. Configure Animation Frames

If using custom animations, edit `src/client/animation/AnimationController.h`:

```cpp
void setupDefaultClips() {
    // Adjust these frame ranges to match your animation export
    m_clips[AnimationType::IDLE] = AnimationClip("Idle", 0, 60, 1.0f, true);
    m_clips[AnimationType::WALK] = AnimationClip("Walk", 61, 91, 1.0f, true);
    m_clips[AnimationType::RUN] = AnimationClip("Run", 92, 112, 1.5f, true);
    // ... etc
}
```

**Frame ranges depend on your animation export:**
- Blender: Check timeline range
- Mixamo: Typically 0-30 for most animations
- Custom: Check your 3D software's export settings

### 4. Rebuild and Run

```cmd
# Rebuild the client
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Release /p:Platform=x64

# Run with your custom models
x64\Release\ExtractionShooterRaylib.exe
```

The game will automatically detect and load your custom models!

---

## Adding Sound Effects (Optional)

Place `.wav` files in the `resources/sounds/` directory:

```
resources/
  sounds/
    ak47_shot.wav
    pistol_shot.wav
    footstep.wav
    loot_pickup.wav
    menu_click.wav
    hurt.wav
    death.wav
```

The sound system will automatically load these files. If not found, it generates placeholder sounds.

**Recommended audio specs:**
- **Format**: WAV (16-bit, 44.1kHz)
- **Length**: 0.1-1.0 seconds for SFX
- **Volume**: Normalized to -6dB to prevent clipping

---

## Free Asset Resources

Don't have 3D models or sounds? Here are free resources:

### 3D Models
- **Mixamo**: https://www.mixamo.com/ (Free rigged characters + animations)
- **Sketchfab**: https://sketchfab.com/features/free-3d-models
- **Poly Pizza**: https://poly.pizza/
- **Quaternius**: https://quaternius.com/index.html (CC0 game assets)

### Sounds
- **Freesound**: https://freesound.org/
- **OpenGameArt**: https://opengameart.org/
- **Sonniss**: https://sonniss.com/gameaudiogdc (Free yearly GDC bundle)
- **ZapSplat**: https://www.zapsplat.com/

### How to Use Mixamo Models

1. Go to https://www.mixamo.com/
2. Select a character (search "soldier" or "shooter")
3. Download as **FBX for Unity**
4. Open in Blender
5. Export as **glTF 2.0 (.glb)**
6. Place in `resources/models/player.glb`

For animations:
1. Select same character on Mixamo
2. Choose animations: Idle, Walk, Run, Shoot, Die, etc.
3. Download each with **"In Place"** option enabled
4. Import all to Blender
5. Combine into single glTF export with all animations

---

## NAT Punchthrough Setup (Advanced)

For players behind routers/firewalls to connect directly:

### 1. Run NAT Server (on public server)

```cmd
NatPunchServer.exe 3478
```

### 2. Connect Clients with NAT

```cmd
ExtractionShooterRaylib.exe --nat --server <public_server_ip>
```

NAT punchthrough enables:
- Direct P2P connections
- Lower latency
- Voice chat capability (future feature)

---

## Troubleshooting

### Common Issues - Quick Fixes

**"Failed to load model"**
→ The game uses placeholder models automatically. No custom models needed!

**"Failed to connect to server"**
→ Make sure server is running first. Check server console shows: `[Server] Server started on port 7777`

**Build errors / "Cannot find raylib.h"**
→ Run `download_dependencies.bat` first! Then verify with `verify_build_env.bat`

**Low FPS**
→ Use Release build (not Debug) and ensure GPU drivers are up to date

**No Sound**
→ Placeholder sounds are auto-generated. Check Windows audio device is working.

### Need More Help?

See **TROUBLESHOOTING.md** for comprehensive solutions to 20+ common issues including:
- Build and compilation errors
- Runtime crashes and errors
- Networking problems
- Performance optimization
- Custom model/animation issues

---

## Next Steps

Now that you're up and running:

1. **Play the game** - Test movement, shooting, looting
2. **Run multiplayer** - Open multiple clients
3. **Add custom models** - Make it your own!
4. **Read the docs** - Check `RAYLIB_README.md` for details
5. **Customize** - Edit source code for your gameplay ideas

## Need Help?

- **Documentation**: See `RAYLIB_README.md`
- **Issues**: Report at GitHub issues page
- **Code**: All source in `src/` directory

---

**Have fun building your extraction shooter!** 🎮
