#!/bin/bash
# Build Environment Verification Script (Linux)

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

ERROR_COUNT=0

echo "========================================"
echo "   Build Environment Verification"
echo "========================================"
echo ""

# Check for CMake
echo -n "[1/8] Checking for CMake... "
if command -v cmake &> /dev/null; then
    CMAKE_VERSION=$(cmake --version | head -n1)
    echo -e "${GREEN}PASS${NC} ($CMAKE_VERSION)"
else
    echo -e "${RED}FAIL${NC} - CMake not found!"
    echo "       Install: sudo apt install cmake"
    ((ERROR_COUNT++))
fi

# Check for Make
echo -n "[2/8] Checking for Make... "
if command -v make &> /dev/null; then
    MAKE_VERSION=$(make --version | head -n1)
    echo -e "${GREEN}PASS${NC} ($MAKE_VERSION)"
else
    echo -e "${RED}FAIL${NC} - Make not found!"
    echo "       Install: sudo apt install build-essential"
    ((ERROR_COUNT++))
fi

# Check for g++
echo -n "[3/8] Checking for g++... "
if command -v g++ &> /dev/null; then
    GCC_VERSION=$(g++ --version | head -n1)
    echo -e "${GREEN}PASS${NC} ($GCC_VERSION)"
else
    echo -e "${RED}FAIL${NC} - g++ not found!"
    echo "       Install: sudo apt install g++"
    ((ERROR_COUNT++))
fi

# Check for raylib
echo -n "[4/8] Checking for raylib... "
if [ -f "dependencies/raylib/include/raylib.h" ] || [ -f "/usr/include/raylib.h" ]; then
    echo -e "${GREEN}PASS${NC}"
else
    echo -e "${RED}FAIL${NC} - raylib not found!"
    echo "       Run: ./download_dependencies.sh"
    ((ERROR_COUNT++))
fi

# Check for raygui
echo -n "[5/8] Checking for raygui... "
if [ -f "dependencies/raygui/raygui.h" ]; then
    echo -e "${GREEN}PASS${NC}"
else
    echo -e "${RED}FAIL${NC} - raygui not found!"
    echo "       Run: ./download_dependencies.sh"
    ((ERROR_COUNT++))
fi

# Check for raylib library
echo -n "[6/8] Checking for raylib library... "
if [ -f "dependencies/raylib/lib/libraylib.so" ] || ldconfig -p | grep -q "libraylib"; then
    echo -e "${GREEN}PASS${NC}"
else
    echo -e "${RED}FAIL${NC} - libraylib.so not found!"
    echo "       Run: ./download_dependencies.sh"
    ((ERROR_COUNT++))
fi

# Check for ENet (optional)
echo -n "[7/8] Checking for ENet... "
if [ -d "dependencies/enet" ]; then
    echo -e "${GREEN}PASS${NC}"
else
    echo -e "${YELLOW}WARN${NC} - ENet not found (optional for NAT punchthrough)"
fi

# Check for CMakeLists.txt
echo -n "[8/8] Checking for CMakeLists.txt... "
if [ -f "CMakeLists.txt" ]; then
    echo -e "${GREEN}PASS${NC}"
else
    echo -e "${RED}FAIL${NC} - CMakeLists.txt missing!"
    ((ERROR_COUNT++))
fi

echo ""
echo "========================================"
echo "   Verification Results"
echo "========================================"
echo ""

if [ $ERROR_COUNT -eq 0 ]; then
    echo -e "${GREEN}[SUCCESS] Environment is ready to build!${NC}"
    echo ""
    echo "You can now build the project:"
    echo "  mkdir -p build && cd build"
    echo "  cmake .."
    echo "  make -j\$(nproc)"
    echo ""
else
    echo -e "${RED}[FAILED] Found $ERROR_COUNT error(s)${NC}"
    echo ""
    echo "Required actions:"
    echo "  1. Install missing dependencies (see above)"
    echo "  2. Run: ./download_dependencies.sh"
    echo "  3. Re-run this verification script"
    echo ""
fi

exit $ERROR_COUNT
