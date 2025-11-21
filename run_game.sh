#!/bin/bash
# Quick launcher for Extraction Shooter with multiple clients (Linux)

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Check if built
if [ ! -f "build/ExtractionShooterServer" ]; then
    echo -e "${RED}Error: Project not built!${NC}"
    echo "Run: ./setup_and_run.sh first"
    exit 1
fi

# Set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$(pwd)/dependencies/raylib/lib:$LD_LIBRARY_PATH

clear
echo "========================================"
echo "  Extraction Shooter - Quick Launcher"
echo "========================================"
echo ""

# Ask for number of clients
echo "How many clients do you want to launch? (1-8)"
read -p "Enter number: " NUM_CLIENTS

# Validate input
if ! [[ "$NUM_CLIENTS" =~ ^[1-8]$ ]]; then
    echo -e "${RED}Invalid number. Using 1 client.${NC}"
    NUM_CLIENTS=1
fi

echo ""
echo -e "${BLUE}Starting server...${NC}"
cd build

# Start server
./ExtractionShooterServer &
SERVER_PID=$!

# Wait for server to start
sleep 2

echo -e "${GREEN}✓ Server started (PID: $SERVER_PID)${NC}"
echo ""

# Start clients
CLIENT_PIDS=()
for ((i=1; i<=NUM_CLIENTS; i++)); do
    echo -e "${BLUE}Starting client $i/$NUM_CLIENTS...${NC}"
    ./ExtractionShooterRaylib &
    CLIENT_PIDS+=($!)
    sleep 1
done

echo ""
echo -e "${GREEN}✓ All clients started!${NC}"
echo ""
echo "Server PID: $SERVER_PID"
echo "Client PIDs: ${CLIENT_PIDS[@]}"
echo ""
echo "========================================"
echo "  Press Enter to stop all processes"
echo "========================================"
read

# Kill all processes
echo ""
echo "Stopping server..."
kill $SERVER_PID 2>/dev/null || true

echo "Stopping clients..."
for pid in "${CLIENT_PIDS[@]}"; do
    kill $pid 2>/dev/null || true
done

# Wait a moment
sleep 1

# Force kill if still running
pkill -f ExtractionShooterServer 2>/dev/null || true
pkill -f ExtractionShooterRaylib 2>/dev/null || true

echo -e "${GREEN}✓ All processes stopped${NC}"
echo ""
