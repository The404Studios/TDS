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

# Download ENet (optional)
echo -e "${YELLOW}[3/3] Downloading ENet...${NC}"
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

cd ..

# Create resource directories
echo ""
echo -e "${YELLOW}Creating resource directories...${NC}"
mkdir -p resources/models
mkdir -p resources/sounds
mkdir -p resources/textures
echo -e "${GREEN}✓ Resource directories created${NC}"

# Create Data directory for server
mkdir -p Data
echo -e "${GREEN}✓ Data directory created${NC}"

echo ""
echo "========================================"
echo -e "${GREEN}  Dependencies Downloaded Successfully!${NC}"
echo "========================================"
echo ""
echo "Next steps:"
echo "  1. Run: ./verify_build_env.sh"
echo "  2. Build: mkdir build && cd build && cmake .. && make"
echo "  3. Run: ./setup_and_run.sh"
echo ""
