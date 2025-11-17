# Troubleshooting Guide

Common issues and solutions for Extraction Shooter development and deployment.

---

## Build Errors

### Error: "Cannot open include file: 'raylib.h'"

**Symptoms**:
```
fatal error C1083: Cannot open include file: 'raylib.h': No such file or directory
```

**Cause**: Dependencies not downloaded

**Solution**:
```cmd
# Run the dependency download script
download_dependencies.bat

# Then verify
verify_build_env.bat
```

---

### Error: Thousands of math/STL errors (`acosf`, `abs`, `sqrt`, etc.)

**Symptoms**:
```
error C3861: 'acosf': identifier not found
error C2065: 'sqrt': undeclared identifier
error C2039: 'allocator_traits': is not a member of 'std'
```

**Cause**: Cascading errors from missing raylib.h - when the header is missing, many template and standard library errors appear

**Solution**: Same as above - download dependencies

---

### Error: "'cout': is not a member of 'std'"

**Symptoms**:
```
error C2039: 'cout': is not a member of 'std'
error C2039: 'endl': is not a member of 'std'
```

**Cause**: Missing `#include <iostream>` in some files

**Solution**: This has been fixed in the codebase. If you still see it, ensure you have the latest version:
```bash
git pull origin main
```

---

### Error: Building entire solution fails

**Symptoms**: Old `ExtractionShooterClient` project fails with hundreds of engine-related errors

**Cause**: The old OpenGL client has incomplete engine stubs and should not be built

**Solution**: Build only the new raylib projects:
```cmd
# Build only these projects:
msbuild ExtractionShooterServer.vcxproj /p:Configuration=Release /p:Platform=x64
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Release /p:Platform=x64
msbuild NatPunchServer.vcxproj /p:Configuration=Release /p:Platform=x64

# Or use the automated script:
setup_and_run.bat
```

**In Visual Studio**:
- Right-click `ExtractionShooterRaylib` → Build
- Right-click `ExtractionShooterServer` → Build
- Do NOT use "Build Solution"

---

### Error: "MSBuild is not recognized"

**Symptoms**:
```
'msbuild' is not recognized as an internal or external command
```

**Cause**: Visual Studio Build Tools not installed or not in PATH

**Solution**:

**Option 1**: Add MSBuild to PATH
```cmd
# Add this to your PATH environment variable:
C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin
```

**Option 2**: Use Developer Command Prompt
- Start Menu → Visual Studio 2022 → Developer Command Prompt for VS 2022
- Run build commands from there

**Option 3**: Install Visual Studio 2022
- Download from: https://visualstudio.microsoft.com/
- Install "Desktop development with C++" workload

---

### Error: raylib.lib not found during linking

**Symptoms**:
```
LINK : fatal error LNK1181: cannot open input file 'raylib.lib'
```

**Cause**: raylib library not in expected location

**Solution**:
```cmd
# Ensure library exists
dir dependencies\raylib\lib\raylib.lib

# If missing, re-download dependencies
download_dependencies.bat

# Verify structure:
dependencies\
  raylib\
    include\
      raylib.h
    lib\
      raylib.lib  <-- Must exist
```

---

## Runtime Errors

### Error: "Server failed to bind to port 7777"

**Symptoms**:
```
[ERROR] Failed to bind to port 7777
[ERROR] Server failed to start
```

**Cause**: Port 7777 already in use or blocked by firewall

**Solution 1**: Kill existing process
```cmd
# Find process using port 7777
netstat -ano | findstr :7777

# Kill it (replace <PID> with actual PID)
taskkill /PID <PID> /F
```

**Solution 2**: Change port in server_config.ini
```ini
[Network]
Port=7778  # Use different port
```

**Solution 3**: Allow through firewall
```cmd
netsh advfirewall firewall add rule name="Extraction Shooter" dir=in action=allow protocol=TCP localport=7777
```

---

### Error: "Failed to connect to server"

**Symptoms**: Client shows "Connection failed" or hangs on login screen

**Possible Causes & Solutions**:

**1. Server not running**
```cmd
# Start server first
x64\Release\ExtractionShooterServer.exe
# Wait for "Server started on port 7777"
# Then start client
```

**2. Wrong server IP**
- Default is 127.0.0.1 (localhost)
- For LAN: Use server's local IP (e.g., 192.168.1.100)
- Edit in src/client/RaylibMain.cpp:
```cpp
const char* GAME_SERVER_IP = "192.168.1.100";  // Change this
```

**3. Firewall blocking**
- Disable firewall temporarily to test
- Or add firewall rule (see above)

**4. Server crashed**
- Check server console for errors
- Check server.log file

---

### Error: Missing DLL errors on launch

**Symptoms**:
```
The code execution cannot proceed because raylib.dll was not found
```

**Cause**: raylib DLL not in executable directory or PATH

**Solution**:
```cmd
# Copy raylib DLL to output directory
copy dependencies\raylib\lib\raylib.dll x64\Release\
copy dependencies\raylib\lib\raylib.dll x64\Debug\
```

Or add to PATH:
```cmd
set PATH=%PATH%;%CD%\dependencies\raylib\lib
```

---

### Error: Black screen / game window doesn't open

**Symptoms**: Client starts but shows only black screen or window doesn't appear

**Possible Causes & Solutions**:

**1. Graphics driver issue**
- Update GPU drivers (NVIDIA/AMD/Intel)
- Ensure OpenGL 3.3+ support

**2. Display scaling issue (Windows)**
- Right-click ExtractionShooterRaylib.exe → Properties → Compatibility
- Check "Disable display scaling on high DPI settings"

**3. Check logs**
```cmd
# Client should output to console
# Look for raylib initialization errors
```

---

### Error: Low FPS / Performance issues

**Symptoms**: Game runs below 60 FPS

**Solutions**:

**1. Build in Release mode** (not Debug)
```cmd
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Release /p:Platform=x64
```

**2. Reduce visual settings** (edit RaylibGameClient.cpp)
```cpp
// Reduce shadow quality or particle count
```

**3. Close other applications**
- Check Task Manager for CPU/GPU usage

**4. Update drivers**
- GPU drivers
- Windows updates

---

### Error: Audio not working

**Symptoms**: No sound in game

**Solutions**:

**1. Check audio device**
- Ensure speakers/headphones connected
- Volume not muted

**2. raylib audio initialization**
- Check console for "Failed to initialize audio device"
- Some systems may not support raylib's audio backend

**3. Fallback to placeholder sounds**
- Game will auto-generate placeholder sounds if files missing
- Check resources/sounds/ directory exists

---

## Networking Issues

### Issue: High ping / latency

**Symptoms**: Laggy player movement, delayed actions

**Solutions**:

**1. Use wired connection** (not WiFi)

**2. Check network quality**
```cmd
ping <server_ip>
# Should be <50ms for good gameplay
```

**3. Reduce network load**
- Close bandwidth-heavy applications (streaming, downloads)
- Limit other devices on network

**4. Server location**
- Use server geographically close to players
- Consider dedicated gaming server providers

---

### Issue: NAT punchthrough not working

**Symptoms**: Can't connect to players through internet (only LAN works)

**Solutions**:

**1. Ensure NAT server running**
```cmd
x64\Release\NatPunchServer.exe
# Should show "NAT Punchthrough Server started on UDP port 3478"
```

**2. Enable NAT in server config**
```ini
[Network]
EnableNAT=true
NATServerIP=<public_nat_server_ip>
NATServerPort=3478
```

**3. Port forwarding** (alternative to NAT punchthrough)
- Forward TCP 7777 on router to server PC
- Find guide for your router model

**4. Symmetric NAT issue**
- Some ISP routers use Symmetric NAT which prevents punchthrough
- Solution: Use VPN or direct port forwarding

---

## Data / Account Issues

### Issue: Lost player progress

**Symptoms**: Account reset, items disappeared

**Solutions**:

**1. Check Data directory**
```cmd
dir Data\
# Should contain player .dat files
```

**2. Restore from backup**
```cmd
# If you ran backups (see DEPLOYMENT_GUIDE.md)
xcopy Backups\Data_<timestamp>\* Data\ /E /Y
```

**3. Server running from different directory**
- Ensure server runs from correct location
- Data\ directory should be in same folder as .exe

---

### Issue: Can't create account / login fails

**Symptoms**: "Registration failed" or "Invalid credentials"

**Solutions**:

**1. Server not running**
- Start server first

**2. Data directory permissions**
```cmd
# Ensure Data\ directory is writable
icacls Data /grant Users:F
```

**3. Check server logs**
```cmd
type server.log
# Look for AuthManager errors
```

**4. Username restrictions**
- 3-20 characters
- Alphanumeric only (no special characters)

---

## Development Issues

### Issue: Changes not taking effect

**Symptoms**: Code changes don't appear in game

**Solution**: Rebuild project
```cmd
# Clean previous build
rd /s /q x64

# Rebuild
msbuild ExtractionShooterRaylib.vcxproj /p:Configuration=Release /p:Platform=x64
```

---

### Issue: Visual Studio IntelliSense errors but builds fine

**Symptoms**: Red underlines in Visual Studio but compilation succeeds

**Solutions**:

**1. Refresh IntelliSense**
- Right-click project → Rescan Solution
- Or: Delete .vs folder and reopen solution

**2. Check include paths**
- Project Properties → C/C++ → General → Additional Include Directories
- Should include: `dependencies\raylib\include` and `dependencies\raygui`

---

### Issue: Git conflicts after pulling updates

**Solution**:
```bash
# Discard local changes and use remote version
git reset --hard origin/main

# Or merge manually
git mergetool
```

---

## Platform-Specific Issues

### Linux / WSL Issues

**Note**: This project is designed for Windows (Visual Studio, raylib Windows builds)

**To run on Linux**:
1. Install raylib for Linux: `sudo apt install libraylib-dev`
2. Use Makefile or CMake instead of Visual Studio
3. Replace Winsock code with POSIX sockets

**Windows Subsystem for Linux (WSL)**:
- Cannot run GUI applications in WSL1
- WSL2 with WSLg may work but not officially supported
- Recommended: Use Windows natively for development

---

## Getting Help

If your issue isn't listed here:

1. **Check server logs**: `server.log` in server directory
2. **Check client console output**: Look for error messages
3. **Verify environment**: Run `verify_build_env.bat`
4. **Search existing issues**: GitHub Issues page
5. **Report bug**: Create detailed issue report with:
   - Full error message
   - Steps to reproduce
   - System specs (OS, CPU, GPU, RAM)
   - Relevant log files

---

## Quick Diagnostic Commands

```cmd
:: Check if server is running
tasklist | findstr ExtractionShooterServer

:: Check port status
netstat -ano | findstr :7777

:: Check dependencies
dir dependencies\raylib\include\raylib.h
dir dependencies\raylib\lib\raylib.lib

:: Check build outputs
dir x64\Release\ExtractionShooterServer.exe
dir x64\Release\ExtractionShooterRaylib.exe

:: View recent logs
type server.log | more

:: Check system resources
wmic cpu get loadpercentage
wmic OS get FreePhysicalMemory
```

---

## Prevention Checklist

Before starting development:

- [ ] Run `verify_build_env.bat`
- [ ] Build in Release mode for testing
- [ ] Keep backups of Data\ directory
- [ ] Test after every major change
- [ ] Use version control (git)
- [ ] Read relevant documentation first

---

**Still stuck?** Check the other guides:
- `BUILD_INSTRUCTIONS.md` - Build process
- `MULTIPLAYER_TESTING_GUIDE.md` - Testing multiplayer
- `DEPLOYMENT_GUIDE.md` - Server deployment
- `QUICK_START_GUIDE.md` - Getting started
