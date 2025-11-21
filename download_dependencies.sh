#!/bin/bash
# Download dependencies for Extraction Shooter (Linux)

set -e  # Exit on error

echo "========================================"
echo "  Downloading Dependencies (Linux)"
echo "========================================"
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Create dependencies directory
mkdir -p dependencies
cd dependencies

# Download raylib
echo -e "${YELLOW}[1/3] Downloading raylib 5.0...${NC}"
if [ ! -d "raylib" ]; then
    # Clone raylib from GitHub
    git clone --depth 1 --branch 5.0 https://github.com/raysan5/raylib.git

    # Build raylib
    cd raylib/src
    make PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED

    # Create lib directory and copy
    cd ..
    mkdir -p lib
    cp src/libraylib.so.5.0.0 lib/libraylib.so
    ln -sf libraylib.so lib/libraylib.so.5
    ln -sf libraylib.so lib/libraylib.so.500

    cd ..
    echo -e "${GREEN}✓ raylib downloaded and built${NC}"
else
    echo -e "${GREEN}✓ raylib already exists${NC}"
fi

# Download raygui
echo -e "${YELLOW}[2/3] Downloading raygui...${NC}"
if [ ! -d "raygui" ]; then
    mkdir -p raygui
    cd raygui

    # Download raygui header
    wget -q https://raw.githubusercontent.com/raysan5/raygui/master/src/raygui.h

    cd ..
    echo -e "${GREEN}✓ raygui downloaded${NC}"
else
    echo -e "${GREEN}✓ raygui already exists${NC}"
fi

# Download ENet
echo -e "${YELLOW}[3/7] Downloading ENet...${NC}"
if [ ! -d "enet" ]; then
    # Clone ENet
    git clone --depth 1 https://github.com/lsalzman/enet.git

    cd enet
    autoreconf -vfi || true
    ./configure --prefix=$(pwd)/install
    make
    make install

    cd ..
    echo -e "${GREEN}✓ ENet downloaded and built${NC}"
else
    echo -e "${GREEN}✓ ENet already exists${NC}"
fi

# Download ImGui
echo -e "${YELLOW}[4/7] Downloading ImGui...${NC}"
if [ ! -d "imgui" ]; then
    git clone --depth 1 --branch docking https://github.com/ocornut/imgui.git
    echo -e "${GREEN}✓ ImGui downloaded${NC}"
else
    echo -e "${GREEN}✓ ImGui already exists${NC}"
fi

# Download nlohmann/json
echo -e "${YELLOW}[5/7] Downloading nlohmann/json...${NC}"
if [ ! -d "json" ]; then
    mkdir -p json
    cd json
    wget -q https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
    mkdir -p nlohmann
    mv json.hpp nlohmann/
    cd ..
    echo -e "${GREEN}✓ nlohmann/json downloaded${NC}"
else
    echo -e "${GREEN}✓ nlohmann/json already exists${NC}"
fi

# Download spdlog
echo -e "${YELLOW}[6/7] Downloading spdlog...${NC}"
if [ ! -d "spdlog" ]; then
    git clone --depth 1 --branch v1.x https://github.com/gabime/spdlog.git
    cd spdlog
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)
    cd ../..
    echo -e "${GREEN}✓ spdlog downloaded and built${NC}"
else
    echo -e "${GREEN}✓ spdlog already exists${NC}"
fi

# Download LZ4
echo -e "${YELLOW}[7/7] Downloading LZ4...${NC}"
if [ ! -d "lz4" ]; then
    git clone --depth 1 https://github.com/lz4/lz4.git
    cd lz4
    make
    cd ..
    echo -e "${GREEN}✓ LZ4 downloaded and built${NC}"
else
    echo -e "${GREEN}✓ LZ4 already exists${NC}"
fi

cd ..

# Create resource directories
echo ""
echo -e "${YELLOW}Creating resource directories...${NC}"
mkdir -p resources/models
mkdir -p resources/sounds
mkdir -p resources/textures
mkdir -p resources/shaders
mkdir -p resources/animations
echo -e "${GREEN}✓ Resource directories created${NC}"

# Create Data directory for server
mkdir -p Data
echo -e "${GREEN}✓ Data directory created${NC}"

echo ""
echo "========================================"
echo -e "${GREEN}  Dependencies Downloaded Successfully!${NC}"
echo "========================================"
echo ""
echo "Dependencies installed:"
echo "  - raylib 5.0 (3D graphics engine)"
echo "  - raygui (UI library)"
echo "  - ENet (NAT punchthrough)"
echo "  - ImGui (Advanced debugging UI)"
echo "  - nlohmann/json (JSON configuration)"
echo "  - spdlog (Professional logging)"
echo "  - LZ4 (Fast compression)"
echo ""
echo "Next steps:"
echo "  1. Run: ./verify_build_env.sh"
echo "  2. Build: mkdir build && cd build && cmake .. && make"
echo "  3. Run: ./setup_and_run.sh"
echo ""
