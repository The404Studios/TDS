#!/bin/bash
# Developer Tools Menu (Linux)

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Set LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$(pwd)/dependencies/raylib/lib:$LD_LIBRARY_PATH

show_menu() {
    clear
    echo -e "${CYAN}========================================"
    echo "   Developer Tools Menu"
    echo "========================================${NC}"
    echo ""
    echo "  Build Options:"
    echo "    1) Build (Debug)"
    echo "    2) Build (Release)"
    echo "    3) Clean build directory"
    echo "    4) Rebuild from scratch"
    echo ""
    echo "  Run Options:"
    echo "    5) Run server"
    echo "    6) Run client"
    echo "    7) Run NAT server"
    echo "    8) Run server + client"
    echo ""
    echo "  Maintenance:"
    echo "    9) View server logs"
    echo "   10) Clear server data"
    echo "   11) Verify build environment"
    echo "   12) Download dependencies"
    echo ""
    echo "   0) Exit"
    echo ""
    echo -e "${CYAN}========================================${NC}"
}

build_debug() {
    echo -e "${BLUE}Building in Debug mode...${NC}"
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug ..
    make -j$(nproc)
    cd ..
    echo -e "${GREEN}✓ Debug build complete${NC}"
    read -p "Press Enter to continue..."
}

build_release() {
    echo -e "${BLUE}Building in Release mode...${NC}"
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)
    cd ..
    echo -e "${GREEN}✓ Release build complete${NC}"
    read -p "Press Enter to continue..."
}

clean_build() {
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build
    echo -e "${GREEN}✓ Build directory cleaned${NC}"
    read -p "Press Enter to continue..."
}

rebuild() {
    echo -e "${YELLOW}Rebuilding from scratch...${NC}"
    rm -rf build
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    make -j$(nproc)
    cd ..
    echo -e "${GREEN}✓ Rebuild complete${NC}"
    read -p "Press Enter to continue..."
}

run_server() {
    if [ ! -f "build/ExtractionShooterServer" ]; then
        echo -e "${RED}Server not built! Build first.${NC}"
        read -p "Press Enter to continue..."
        return
    fi

    echo -e "${BLUE}Starting server...${NC}"
    echo "Press Ctrl+C to stop"
    echo ""
    cd build
    ./ExtractionShooterServer
    cd ..
    read -p "Press Enter to continue..."
}

run_client() {
    if [ ! -f "build/ExtractionShooterRaylib" ]; then
        echo -e "${RED}Client not built! Build first.${NC}"
        read -p "Press Enter to continue..."
        return
    fi

    echo -e "${BLUE}Starting client...${NC}"
    echo "Press Ctrl+C to stop"
    echo ""
    cd build
    ./ExtractionShooterRaylib
    cd ..
    read -p "Press Enter to continue..."
}

run_nat_server() {
    if [ ! -f "build/NatPunchServer" ]; then
        echo -e "${RED}NAT server not built! Build first.${NC}"
        read -p "Press Enter to continue..."
        return
    fi

    echo -e "${BLUE}Starting NAT punchthrough server...${NC}"
    echo "Press Ctrl+C to stop"
    echo ""
    cd build
    ./NatPunchServer
    cd ..
    read -p "Press Enter to continue..."
}

run_both() {
    if [ ! -f "build/ExtractionShooterServer" ] || [ ! -f "build/ExtractionShooterRaylib" ]; then
        echo -e "${RED}Server or client not built! Build first.${NC}"
        read -p "Press Enter to continue..."
        return
    fi

    echo -e "${BLUE}Starting server...${NC}"
    cd build
    ./ExtractionShooterServer &
    SERVER_PID=$!
    sleep 2

    echo -e "${BLUE}Starting client...${NC}"
    ./ExtractionShooterRaylib &
    CLIENT_PID=$!

    echo ""
    echo -e "${GREEN}Server and client started${NC}"
    echo "Server PID: $SERVER_PID"
    echo "Client PID: $CLIENT_PID"
    echo ""
    read -p "Press Enter to stop both..."

    kill $SERVER_PID 2>/dev/null || true
    kill $CLIENT_PID 2>/dev/null || true

    cd ..
}

view_logs() {
    if [ -f "build/server.log" ]; then
        echo -e "${BLUE}Server logs (last 50 lines):${NC}"
        echo ""
        tail -50 build/server.log
    else
        echo -e "${YELLOW}No server logs found${NC}"
    fi
    echo ""
    read -p "Press Enter to continue..."
}

clear_data() {
    read -p "Are you sure you want to clear all server data? (y/n): " -n 1 -r
    echo ""
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        rm -rf Data/*.dat
        rm -f build/server.log
        echo -e "${GREEN}✓ Server data cleared${NC}"
    else
        echo "Cancelled."
    fi
    read -p "Press Enter to continue..."
}

verify_env() {
    ./verify_build_env.sh
    read -p "Press Enter to continue..."
}

download_deps() {
    ./download_dependencies.sh
    read -p "Press Enter to continue..."
}

# Main loop
while true; do
    show_menu
    read -p "Select option: " choice

    case $choice in
        1) build_debug ;;
        2) build_release ;;
        3) clean_build ;;
        4) rebuild ;;
        5) run_server ;;
        6) run_client ;;
        7) run_nat_server ;;
        8) run_both ;;
        9) view_logs ;;
        10) clear_data ;;
        11) verify_env ;;
        12) download_deps ;;
        0) echo "Goodbye!"; exit 0 ;;
        *) echo -e "${RED}Invalid option${NC}"; sleep 1 ;;
    esac
done
