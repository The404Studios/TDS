#!/bin/bash

# TDS - IDE Cache Cleanup Script
# Run this script to completely clean all IDE caches and force a fresh reindex

echo "=========================================="
echo "TDS - IDE Cache Cleanup"
echo "=========================================="
echo ""

cd "$(dirname "$0")"

echo "1. Removing Visual Studio cache..."
rm -rf .vs/
rm -rf out/
rm -rf x64/
rm -rf x86/
rm -rf Debug/
rm -rf Release/
rm -f *.suo
rm -f *.user
rm -f *.userosscache
rm -f *.sln.docstates
rm -f *.VC.db
rm -f *.VC.opendb
echo "   ✓ Visual Studio cache removed"

echo ""
echo "2. Removing VS Code cache..."
rm -rf .vscode/
echo "   ✓ VS Code cache removed"

echo ""
echo "3. Removing JetBrains (CLion, etc.) cache..."
rm -rf .idea/
rm -rf cmake-build-*/
echo "   ✓ JetBrains cache removed"

echo ""
echo "4. Removing CMake cache..."
rm -rf build/CMakeFiles/
rm -f build/CMakeCache.txt
rm -f build/*.cmake
echo "   ✓ CMake cache removed"

echo ""
echo "5. Regenerating compile_commands.json..."
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .. > /dev/null 2>&1
cd ..
echo "   ✓ compile_commands.json regenerated"

echo ""
echo "=========================================="
echo "✓ IDE cache cleanup complete!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. Close your IDE completely if it's still open"
echo "2. Reopen the PROJECT FOLDER (not a .sln file)"
echo "3. Let it detect CMakeLists.txt"
echo "4. Wait for indexing to complete"
echo ""
echo "If using Visual Studio:"
echo "  - Use 'Open > Folder' NOT 'Open > Project/Solution'"
echo "  - Point to: $(pwd)"
echo ""
echo "All errors for SceneManager, UIButton, WeatherSystem"
echo "should disappear after reindexing."
echo ""
