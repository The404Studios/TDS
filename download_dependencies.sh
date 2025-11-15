#!/bin/bash

echo "==================================================="
echo "  Downloading TDS Dependencies"
echo "==================================================="

# ============================================================================
# ENet
# ============================================================================
echo ""
echo "Downloading ENet..."
cd external/enet

if [ ! -f "host.c" ]; then
    # Download ENet 1.3.17 sources
    wget -q http://enet.bespin.org/download/enet-1.3.17.tar.gz -O enet.tar.gz
    tar -xzf enet.tar.gz --strip-components=1
    rm enet.tar.gz
    echo "✅ ENet downloaded"
else
    echo "✅ ENet already exists"
fi

cd ../..

# ============================================================================
# SQLite
# ============================================================================
echo ""
echo "Downloading SQLite..."
cd external/sqlite

if [ ! -f "sqlite3.c" ]; then
    # Download SQLite amalgamation
    wget -q https://www.sqlite.org/2024/sqlite-amalgamation-3450000.zip -O sqlite.zip
    unzip -q sqlite.zip
    mv sqlite-amalgamation-*/* .
    rm -rf sqlite-amalgamation-* sqlite.zip
    echo "✅ SQLite downloaded"
else
    echo "✅ SQLite already exists"
fi

cd ../..

# ============================================================================
# Free Game Assets
# ============================================================================
echo ""
echo "Setting up asset placeholders..."

# Create placeholder 3D models
cat > assets/models/placeholder.obj << 'EOF'
# Placeholder cube
v -0.5 -0.5  0.5
v  0.5 -0.5  0.5
v -0.5  0.5  0.5
v  0.5  0.5  0.5
v -0.5  0.5 -0.5
v  0.5  0.5 -0.5
v -0.5 -0.5 -0.5
v  0.5 -0.5 -0.5

f 1 2 4 3
f 3 4 6 5
f 5 6 8 7
f 7 8 2 1
f 2 8 6 4
f 7 1 3 5
EOF

echo "✅ Asset placeholders created"

# Print asset download suggestions
echo ""
echo "==================================================="
echo "  Free Asset Resources"
echo "==================================================="
echo ""
echo "3D Models:"
echo "  - Kenney.nl: https://kenney.nl/assets"
echo "  - Quaternius: https://quaternius.com/index.html"
echo "  - Sketchfab (CC0): https://sketchfab.com/search?features=downloadable&licenses=322a749bcfa841b29dff1e8a1bb74b0b&type=models"
echo ""
echo "Sounds:"
echo "  - Freesound.org: https://freesound.org/"
echo "  - OpenGameArt.org: https://opengameart.org/"
echo "  - BBC Sound Effects: https://sound-effects.bbcrewind.co.uk/"
echo ""
echo "Textures:"
echo "  - PolyHaven: https://polyhaven.com/textures"
echo "  - AmbientCG: https://ambientcg.com/"
echo ""
echo "==================================================="
echo "  Dependencies Ready!"
echo "==================================================="
echo ""
echo "You can now build the project:"
echo "  cmake -B build -DCMAKE_BUILD_TYPE=Release"
echo "  cmake --build build -j\$(nproc)"
echo ""
