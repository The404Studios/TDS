# Kali Linux Quick Start Guide

Get Extraction Shooter running on Kali Linux in under 10 minutes!

---

## Prerequisites

### System Requirements
- **Kali Linux** 2020.1 or newer (tested on 2023.4)
- **RAM**: 4 GB minimum, 8 GB recommended
- **Disk**: 2 GB free space
- **Display**: X11 server (graphical environment)

### Check Your System

```bash
# Check Kali version
cat /etc/os-release | grep VERSION

# Check if running X server
echo $DISPLAY
# Should output: :0 or similar

# Check available disk space
df -h ~
```

---

## Step 1: Install Dependencies

Open a terminal and run:

```bash
# Update package list
sudo apt update

# Install build tools and libraries
sudo apt install -y \
    build-essential \
    cmake \
    git \
    wget \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libxinerama-dev \
    libxcursor-dev \
    libgl1-mesa-dev \
    libasound2-dev
```

**This takes ~2-3 minutes on Kali.**

---

## Step 2: Download Project

If you haven't already:

```bash
# Clone the repository
git clone <repository-url>
cd TDS
```

Or if you already have it:

```bash
cd TDS
git pull origin main
```

---

## Step 3: Download Game Dependencies

```bash
# Run the dependency download script
./download_dependencies.sh
```

This will:
- ✅ Clone and build raylib 5.0 (~2 minutes)
- ✅ Download raygui
- ✅ Build ENet for NAT punchthrough
- ✅ Create resource directories

**Total time: ~3-5 minutes**

---

## Step 4: Build the Game

### Option A: Automated (Recommended)

```bash
./setup_and_run.sh
```

This will:
1. Verify environment
2. Build with CMake
3. Launch the game automatically

**Just press 'y' when prompted to launch!**

### Option B: Manual Build

```bash
# Create build directory
mkdir -p build
cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build (uses all CPU cores)
make -j$(nproc)

# Back to project root
cd ..
```

---

## Step 5: Run the Game

### First Time Launch

```bash
# Set library path
export LD_LIBRARY_PATH=$(pwd)/dependencies/raylib/lib:$LD_LIBRARY_PATH

# Start server (Terminal 1)
cd build
./ExtractionShooterServer
```

Open another terminal:

```bash
# Set library path
export LD_LIBRARY_PATH=$(pwd)/dependencies/raylib/lib:$LD_LIBRARY_PATH

# Start client (Terminal 2)
cd build
./ExtractionShooterRaylib
```

### Quick Launch (After First Build)

```bash
./run_game.sh
# Enter: 1 (for single player)
```

---

## Gameplay Quick Guide

### Login Screen
1. Enter any username and password
2. Click "Register" (creates new account)
3. Click "Login"

### Controls
- **WASD** - Move
- **Shift** - Sprint
- **Mouse** - Look around
- **Left Click** - Shoot
- **R** - Reload
- **F** - Loot corpse
- **Tab** - Inventory
- **Escape** - Menu/Back

Full controls: [CONTROLS.md](CONTROLS.md)

---

## Troubleshooting

### "cmake: command not found"

```bash
sudo apt install cmake
```

### "Cannot find raylib.h"

```bash
# Re-run dependency download
./download_dependencies.sh
```

### "error while loading shared libraries: libraylib.so"

```bash
# Run this in every terminal before launching
export LD_LIBRARY_PATH=$(pwd)/dependencies/raylib/lib:$LD_LIBRARY_PATH

# Or add to ~/.bashrc for permanent fix:
echo 'export LD_LIBRARY_PATH=/path/to/TDS/dependencies/raylib/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### "Failed to initialize GLFW"

Make sure you're running in a graphical environment (not SSH without X forwarding).

```bash
# Check display
echo $DISPLAY

# If empty, you're in console mode - switch to GUI
startx
```

### "Server failed to bind to port 7777"

Port is already in use:

```bash
# Find and kill process using port
sudo netstat -tulpn | grep 7777
sudo kill -9 <PID>
```

### More Issues?

See comprehensive troubleshooting: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)

---

## Performance Tips for Kali

### Running on VM?

If running Kali in VirtualBox/VMware:

1. **Enable 3D Acceleration**:
   - VirtualBox: Settings → Display → Enable 3D Acceleration
   - VMware: Settings → Display → Accelerate 3D graphics

2. **Allocate More RAM**:
   - Give VM at least 4 GB (8 GB recommended)

3. **Install Guest Additions** (VirtualBox):
   ```bash
   sudo apt install virtualbox-guest-x11
   ```

### Optimize Build Speed

```bash
# Use all CPU cores
make -j$(nproc)

# Use ccache for faster rebuilds
sudo apt install ccache
cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ..
```

---

## Multiplayer Testing on Kali

### Test with Multiple Clients

```bash
./run_game.sh
# Enter: 4 (for 4 clients)
```

All clients will connect to localhost server.

### LAN Multiplayer

**On Server Machine** (Kali):
```bash
# Find your IP
ip addr show | grep inet

# Start server
cd build
./ExtractionShooterServer
```

**On Client Machines**:
- Edit `src/client/RaylibMain.cpp`
- Change `GAME_SERVER_IP` to server's IP
- Rebuild and run

---

## Development on Kali

### Quick Development Workflow

```bash
# Use interactive dev tools
./dev_tools.sh

# Menu options:
# 1) Build Debug
# 2) Build Release
# 5) Run server
# 6) Run client
# 9) View logs
```

### Debugging

```bash
# Install GDB
sudo apt install gdb

# Build in debug mode
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Debug server
gdb ./ExtractionShooterServer
(gdb) run
```

### Memory Leak Detection

```bash
# Install valgrind
sudo apt install valgrind

# Run with valgrind
valgrind --leak-check=full ./build/ExtractionShooterServer
```

---

## Kali-Specific Features

### Penetration Testing Integration

Since you're on Kali, you can use built-in tools to test the game's network security:

```bash
# Monitor game traffic (educational purposes)
sudo wireshark &
# Filter: tcp.port == 7777

# Port scanning
nmap -p 7777 localhost

# Stress testing
hping3 -S -p 7777 localhost
```

**Note**: Only test on your own servers! Unauthorized testing is illegal.

### Firewall Configuration

```bash
# Allow game ports through UFW
sudo ufw allow 7777/tcp  # Game server
sudo ufw allow 3478/udp  # NAT punchthrough

# Or use iptables
sudo iptables -A INPUT -p tcp --dport 7777 -j ACCEPT
sudo iptables -A INPUT -p udp --dport 3478 -j ACCEPT
```

---

## Uninstallation

### Remove Build Only

```bash
rm -rf build
```

### Remove Everything

```bash
cd ..
rm -rf TDS
```

### Remove System Packages

```bash
sudo apt remove --purge \
    build-essential \
    cmake \
    libx11-dev \
    libgl1-mesa-dev \
    libasound2-dev
sudo apt autoremove
```

---

## Next Steps

### For Players
- 📖 Read [CONTROLS.md](CONTROLS.md) - Learn all controls
- 🎮 Read [QUICK_START_GUIDE.md](QUICK_START_GUIDE.md) - Detailed gameplay guide

### For Developers
- 🔧 Read [BUILD_LINUX.md](BUILD_LINUX.md) - Advanced Linux build options
- 💻 Read [CONTRIBUTING.md](CONTRIBUTING.md) - How to contribute
- 📘 Read [RAYLIB_README.md](RAYLIB_README.md) - Technical architecture

### For Server Admins
- 🌐 Read [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) - Deploy to production
- 🧪 Read [MULTIPLAYER_TESTING_GUIDE.md](MULTIPLAYER_TESTING_GUIDE.md) - Test scenarios

---

## Common Kali Linux Commands Cheat Sheet

```bash
# Update Kali
sudo apt update && sudo apt upgrade -y

# Check system info
uname -a
lsb_release -a

# Monitor resources
htop

# Check network
ip addr show
netstat -tulpn

# View processes
ps aux | grep Extraction

# Kill processes
pkill -f ExtractionShooter

# Check disk space
df -h

# Check memory
free -h

# View logs
tail -f build/server.log
```

---

## Success Checklist

After following this guide, you should have:

- [✅] Kali Linux with required packages installed
- [✅] Project cloned and dependencies downloaded
- [✅] Game successfully built
- [✅] Server running on port 7777
- [✅] Client connecting and playable
- [✅] No compilation or runtime errors

---

## Getting Help

**If something doesn't work**:

1. Run verification: `./verify_build_env.sh`
2. Check logs: `cat build/server.log`
3. Read troubleshooting: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
4. Check [BUILD_LINUX.md](BUILD_LINUX.md) for advanced options

**Still stuck?** Open a GitHub issue with:
- Kali version: `cat /etc/os-release`
- Error messages (full output)
- Build log if applicable

---

## Why Kali Linux?

Kali Linux is perfect for this project because:
- ✅ **Up-to-date packages** - Latest compilers and tools
- ✅ **Developer-friendly** - Built for technical users
- ✅ **Extensive tooling** - Perfect for network testing
- ✅ **Debian-based** - Stable and well-supported
- ✅ **Works great in VMs** - Easy to test and deploy

---

**Status**: ✅ Fully tested and working on Kali Linux 2023.4!

**Build time**: ~5-10 minutes (including dependency download)

**Have fun!** 🎮🐧
