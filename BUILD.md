# Building TDS - Tarkov Looter Shooter

## Quick Start

```bash
# 1. Download dependencies
bash download_dependencies.sh

# 2. Configure CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 3. Build
cmake --build build -j$(nproc)

# 4. Run server
./build/TDS_Server

# 5. Run client (in another terminal)
./build/TDS_Client
```

## Prerequisites

### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    libgl1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libxcursor-dev \
    libxinerama-dev \
    libasound2-dev
```

### macOS
```bash
# Install Homebrew if needed: https://brew.sh
brew install cmake
```

### Windows
- Install Visual Studio 2022 (Community Edition)
- Install CMake: https://cmake.org/download/
- Or use MSYS2/MinGW-w64

## Dependencies

All dependencies are automatically downloaded and built:

- **Raylib 5.0** - Fetched via CMake FetchContent
- **ENet 1.3.17** - UDP networking library (vendored)
- **SQLite 3.45** - Database (vendored)

## Build Options

```bash
# Build only client
cmake -B build -DBUILD_CLIENT=ON -DBUILD_SERVER=OFF
cmake --build build

# Build only server
cmake -B build -DBUILD_CLIENT=OFF -DBUILD_SERVER=ON
cmake --build build

# Debug build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Verbose build
cmake --build build --verbose
```

## Platform-Specific Notes

### Linux
- Default build system uses Makefiles
- Supports GCC and Clang
- Audio requires ALSA development headers

### Windows
- Uses Visual Studio generator by default
- Can use MinGW-w64 with `-G "MinGW Makefiles"`
- Winsock2 automatically linked

### macOS
- Uses Unix Makefiles
- Supports both Intel and Apple Silicon
- No additional dependencies needed

## Troubleshooting

### "raylib not found"
The build will automatically download Raylib. Ensure you have internet connection during first build.

### "Cannot find -lenet"
Run `bash download_dependencies.sh` to download ENet sources.

### "SQLite not found"
Run `bash download_dependencies.sh` to download SQLite amalgamation.

### Linking errors
```bash
# Clean build
rm -rf build
cmake -B build
cmake --build build
```

### Raylib display issues on Linux
```bash
# Install OpenGL and X11 dev packages
sudo apt-get install libgl1-mesa-dev libx11-dev
```

## Installation

```bash
# Install to system (requires sudo on Linux/macOS)
cmake --install build --prefix /usr/local

# Or install to custom location
cmake --install build --prefix $HOME/tds

# Uninstall
cmake --build build --target uninstall  # If available
```

## Development

### Generate compile_commands.json
```bash
cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
ln -s build/compile_commands.json .
```

### IDE Setup

**VS Code:**
- Install C/C++ extension
- Install CMake Tools extension
- Open folder and select build kit

**CLion:**
- Open folder
- CLion automatically detects CMake

**Visual Studio:**
- File → Open → CMake
- Select CMakeLists.txt

## Performance

### Optimized Build
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -march=native"
cmake --build build
```

### Profile-Guided Optimization (Advanced)
```bash
# 1. Build with profiling
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-fprofile-generate"
cmake --build build

# 2. Run game to generate profile data
./build/TDS_Client

# 3. Rebuild with profile
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-fprofile-use"
cmake --build build
```

## Testing

```bash
# Run server
cd build
./TDS_Server

# In another terminal, run client
cd build
./TDS_Client

# For multiple clients, run in separate terminals
./TDS_Client &
./TDS_Client &
./TDS_Client &
```

## Asset Management

Place your game assets in:
- `assets/models/` - .obj, .gltf, .glb files
- `assets/textures/` - .png, .jpg files
- `assets/sounds/` - .ogg, .wav files
- `assets/music/` - .ogg, .mp3 files

Assets are automatically copied to build directory.

## Cross-Compilation

### Windows → Linux
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=linux-x86_64.cmake
```

### Linux → Windows (MinGW)
```bash
sudo apt-get install mingw-w64
cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchains/mingw-w64.cmake
```

## License

See LICENSE file for details.

## Support

For issues: https://github.com/The404Studios/TDS/issues
