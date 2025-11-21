# Build Instructions - Linux / Kali Linux

Complete guide for building Extraction Shooter on Linux systems (Debian, Ubuntu, Kali, etc.).

---

## Quick Start

```bash
# 1. Download dependencies
./download_dependencies.sh

# 2. Build and run
./setup_and_run.sh
```

That's it! The game will launch automatically.

---

## System Requirements

### Operating Systems
- ✅ Kali Linux 2020.1+
- ✅ Debian 10+
- ✅ Ubuntu 20.04+
- ✅ Any Linux distribution with recent kernel (4.x+)

### Required Packages

```bash
# For Debian/Ubuntu/Kali
sudo apt update
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

### Optional Packages

```bash
# For better development experience
sudo apt install -y \
    clang \
    gdb \
    valgrind \
    ccache
```

---

## Installation Methods

### Method 1: Automated Setup (Recommended)

```bash
# Step 1: Download dependencies
./download_dependencies.sh

# Step 2: Verify environment
./verify_build_env.sh

# Step 3: Build and run
./setup_and_run.sh
```

### Method 2: Manual Build

```bash
# Step 1: Download dependencies
./download_dependencies.sh

# Step 2: Create build directory
mkdir -p build
cd build

# Step 3: Configure with CMake
cmake -DCMAKE_BUILD_TYPE=Release ..

# Step 4: Build (using all CPU cores)
make -j$(nproc)

# Step 5: Run
./ExtractionShooterServer  # Terminal 1
./ExtractionShooterRaylib  # Terminal 2
```

### Method 3: Using System raylib (If installed)

```bash
# Install raylib from system package manager
sudo apt install libraylib-dev

# Build
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

---

## Build Configuration

### Build Types

**Release (Optimized)**:
```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

**Debug (With symbols)**:
```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
```

**RelWithDebInfo (Optimized + Symbols)**:
```bash
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make -j$(nproc)
```

### Compiler Selection

**Use GCC (default)**:
```bash
cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ ..
```

**Use Clang**:
```bash
cmake -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..
```

### Parallel Builds

```bash
# Use all CPU cores
make -j$(nproc)

# Use specific number of cores
make -j4

# Single-threaded (for debugging build issues)
make
```

---

## Running the Game

### Set Library Path

Before running, ensure raylib can be found:

```bash
# Temporary (current session only)
export LD_LIBRARY_PATH=$(pwd)/dependencies/raylib/lib:$LD_LIBRARY_PATH

# Permanent (add to ~/.bashrc)
echo 'export LD_LIBRARY_PATH=/path/to/TDS/dependencies/raylib/lib:$LD_LIBRARY_PATH' >> ~/.bashrc
source ~/.bashrc
```

### Quick Launch Scripts

**Single client**:
```bash
./run_game.sh
# Enter: 1
```

**Multiple clients (multiplayer testing)**:
```bash
./run_game.sh
# Enter: 4 (for 4 clients)
```

**Developer tools**:
```bash
./dev_tools.sh
# Interactive menu for build, run, debug
```

### Manual Launch

```bash
cd build

# Terminal 1: Start server
./ExtractionShooterServer

# Terminal 2: Start client
./ExtractionShooterRaylib

# Terminal 3: Start NAT server (optional)
./NatPunchServer
```

### Background Execution

```bash
# Run server in background
./ExtractionShooterServer &
SERVER_PID=$!

# Run client
./ExtractionShooterRaylib

# Stop server
kill $SERVER_PID
```

---

## Troubleshooting

### Missing Dependencies

**Error**: `cmake: command not found`

**Solution**:
```bash
sudo apt install cmake
```

---

**Error**: `g++: command not found`

**Solution**:
```bash
sudo apt install build-essential
```

---

**Error**: `Cannot find raylib`

**Solution**:
```bash
./download_dependencies.sh
```

---

### Build Errors

**Error**: `X11/Xlib.h: No such file or directory`

**Solution**:
```bash
sudo apt install libx11-dev
```

---

**Error**: `GL/gl.h: No such file or directory`

**Solution**:
```bash
sudo apt install libgl1-mesa-dev
```

---

**Error**: `undefined reference to pthread_create`

**Solution**: Already handled by CMake. If persists:
```bash
cmake -DCMAKE_EXE_LINKER_FLAGS="-pthread" ..
```

---

### Runtime Errors

**Error**: `error while loading shared libraries: libraylib.so`

**Solution**:
```bash
export LD_LIBRARY_PATH=$(pwd)/dependencies/raylib/lib:$LD_LIBRARY_PATH
```

Or copy library to system path:
```bash
sudo cp dependencies/raylib/lib/libraylib.so.5.0.0 /usr/local/lib/
sudo ldconfig
```

---

**Error**: `Failed to initialize GLFW`

**Solution**: Missing X11 or running in non-graphical environment

```bash
# Check if X server is running
echo $DISPLAY

# If empty, start X server or use SSH with X forwarding
ssh -X user@host
```

---

**Error**: `Server failed to bind to port 7777`

**Solution**:
```bash
# Check if port is in use
sudo netstat -tulpn | grep 7777

# Kill process using port
sudo kill -9 $(sudo lsof -t -i:7777)

# Or use different port (edit server_config.ini)
```

---

### Permission Issues

**Error**: `Permission denied` when running scripts

**Solution**:
```bash
chmod +x *.sh
```

---

**Error**: Can't write to Data/ directory

**Solution**:
```bash
chmod 755 Data
# Or
sudo chown -R $USER:$USER Data
```

---

## Platform-Specific Notes

### Kali Linux

Kali works perfectly out of the box:
```bash
sudo apt update
sudo apt install build-essential cmake git libx11-dev libgl1-mesa-dev libasound2-dev
./setup_and_run.sh
```

### Ubuntu / Debian

Identical to Kali. Use the same commands above.

### Arch Linux

```bash
sudo pacman -S base-devel cmake git libx11 mesa alsa-lib
./setup_and_run.sh
```

### Fedora / RHEL

```bash
sudo dnf groupinstall "Development Tools"
sudo dnf install cmake git libX11-devel mesa-libGL-devel alsa-lib-devel
./setup_and_run.sh
```

---

## Advanced Configuration

### Installation

```bash
# Install to /usr/local
cd build
sudo make install

# Run from anywhere
ExtractionShooterServer
ExtractionShooterRaylib
```

### Custom Install Prefix

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/extraction-shooter ..
make
sudo make install
```

### Static Linking (Portable Binary)

```bash
# Edit CMakeLists.txt, add:
# set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

---

## Performance Optimization

### Compiler Optimizations

```bash
# Maximum optimization
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native" ..

# Link-time optimization
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON ..
```

### Using ccache (Faster Rebuilds)

```bash
# Install ccache
sudo apt install ccache

# Configure CMake to use it
cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache ..
```

---

## Development Workflow

### Quick Development Cycle

```bash
# 1. Make code changes

# 2. Quick rebuild
cd build && make -j$(nproc) && cd ..

# 3. Test
./run_game.sh
```

### Interactive Development

```bash
# Use developer tools
./dev_tools.sh

# Menu options:
# - Quick build (Debug/Release)
# - Run server/client
# - View logs
# - Clear data
```

### Debugging

```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Run in gdb
gdb ./build/ExtractionShooterServer
# (gdb) run

# Run with valgrind
valgrind --leak-check=full ./build/ExtractionShooterServer
```

---

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build Linux

on: [push, pull_request]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v2

    - name: Install dependencies
      run: |
        sudo apt update
        sudo apt install -y build-essential cmake libx11-dev libgl1-mesa-dev libasound2-dev

    - name: Download game dependencies
      run: ./download_dependencies.sh

    - name: Build
      run: |
        mkdir build
        cd build
        cmake -DCMAKE_BUILD_TYPE=Release ..
        make -j$(nproc)

    - name: Test
      run: |
        cd build
        ./ExtractionShooterServer --version || true
```

---

## Uninstallation

### Remove Build

```bash
rm -rf build
```

### Remove Dependencies

```bash
rm -rf dependencies
```

### Remove All Data

```bash
rm -rf Data build dependencies resources
```

### Uninstall System-wide

```bash
cd build
sudo make uninstall
# Or manually:
sudo rm /usr/local/bin/ExtractionShooter*
```

---

## Comparison: Linux vs Windows

| Feature | Linux | Windows |
|---------|-------|---------|
| Build System | CMake + Make | Visual Studio |
| Compiler | GCC/Clang | MSVC |
| Scripts | Shell (.sh) | Batch (.bat) |
| Networking | POSIX sockets | Winsock2 |
| Dependencies | Manual/Package Manager | Automated download |
| Performance | ✅ Native | ✅ Native |

---

## Next Steps

- **Play**: Read [CONTROLS.md](CONTROLS.md) to learn game controls
- **Develop**: Read [CONTRIBUTING.md](CONTRIBUTING.md) for development guide
- **Deploy**: Read [DEPLOYMENT_GUIDE.md](DEPLOYMENT_GUIDE.md) for server hosting
- **Troubleshoot**: Read [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for issue solutions

---

## Getting Help

**For Linux-specific build issues**:

1. Run `./verify_build_env.sh`
2. Check error messages carefully
3. Install missing packages
4. Re-run `./download_dependencies.sh`

**Still stuck?**
- Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- Open GitHub issue with:
  - Linux distribution and version (`lsb_release -a`)
  - Build log output
  - Error messages

---

**Status**: ✅ Fully supported on Linux!
**Tested on**: Kali Linux 2023.4, Ubuntu 22.04, Debian 12
