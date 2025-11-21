#!/bin/bash
# One-click setup, build, and run script (Linux)

set -e  # Exit on error

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

clear
echo "========================================"
echo "  Extraction Shooter - Setup & Run"
echo "========================================"
echo ""

# Step 1: Check dependencies
echo -e "${BLUE}[Step 1/4] Checking dependencies...${NC}"
if [ ! -d "dependencies/raylib" ]; then
    echo -e "${YELLOW}Dependencies not found. Downloading...${NC}"
    ./download_dependencies.sh
else
    echo -e "${GREEN}✓ Dependencies already downloaded${NC}"
fi
echo ""

# Step 2: Verify environment
echo -e "${BLUE}[Step 2/4] Verifying build environment...${NC}"
if ./verify_build_env.sh; then
    echo -e "${GREEN}✓ Environment verification passed${NC}"
else
    echo -e "${RED}✗ Environment verification failed${NC}"
    echo "Please fix the issues above and try again."
    exit 1
fi
echo ""

# Step 3: Build
echo -e "${BLUE}[Step 3/4] Building project...${NC}"

# Create build directory
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
echo "Building (using $(nproc) cores)..."
make -j$(nproc)

cd ..

echo -e "${GREEN}✓ Build complete!${NC}"
echo ""

# Step 4: Run
echo -e "${BLUE}[Step 4/4] Starting game...${NC}"
echo ""

# Set LD_LIBRARY_PATH for raylib
export LD_LIBRARY_PATH=$(pwd)/dependencies/raylib/lib:$LD_LIBRARY_PATH

read -p "Do you want to launch the game now? (y/n): " -n 1 -r
echo ""

if [[ $REPLY =~ ^[Yy]$ ]]; then
    echo ""
    echo "Starting server..."
    cd build

    # Start server in background
    ./ExtractionShooterServer &
    SERVER_PID=$!

    # Wait for server to start
    echo "Waiting for server to initialize..."
    sleep 2

    # Start client
    echo ""
    echo "Starting client..."
    ./ExtractionShooterRaylib &
    CLIENT_PID=$!

    echo ""
    echo -e "${GREEN}Game launched!${NC}"
    echo "Server PID: $SERVER_PID"
    echo "Client PID: $CLIENT_PID"
    echo ""
    echo "Press Enter to stop all processes..."
    read

    # Kill processes
    kill $SERVER_PID 2>/dev/null || true
    kill $CLIENT_PID 2>/dev/null || true

    echo "Stopped."
else
    echo ""
    echo "Build complete! Run manually:"
    echo "  cd build"
    echo "  ./ExtractionShooterServer  # In terminal 1"
    echo "  ./ExtractionShooterRaylib  # In terminal 2"
fi

echo ""
echo "========================================"
echo "  Setup Complete!"
echo "========================================"
