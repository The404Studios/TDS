#ifndef TERRAIN_MANAGER_H
#define TERRAIN_MANAGER_H

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "CivilizationAI.h"

// Terrain types
enum class TerrainType {
    GRASS,
    DIRT,
    SAND,
    STONE,
    WATER,
    SNOW
};

// Terrain tile
struct TerrainTile {
    float height;
    TerrainType type;
    Vector3 normal;
    float moisture;
    float temperature;

    TerrainTile() : height(0), type(TerrainType::GRASS), normal(0, 0, 1),
                    moisture(0.5f), temperature(0.5f) {}
};

// Biome data
struct Biome {
    TerrainType type;
    float minHeight;
    float maxHeight;
    float minMoisture;
    float maxMoisture;
    float r, g, b; // Color

    Biome(TerrainType t, float minh, float maxh, float minm, float maxm,
          float red, float green, float blue)
        : type(t), minHeight(minh), maxHeight(maxh),
          minMoisture(minm), maxMoisture(maxm), r(red), g(green), b(blue) {}
};

class TerrainManager {
private:
    int width;
    int depth;
    float tileSize;
    std::vector<std::vector<TerrainTile>> tiles;
    std::vector<Biome> biomes;

    // Perlin noise for terrain generation
    float perlinNoise(float x, float y, int octaves, float persistence);
    float interpolate(float a, float b, float t);
    float noise2D(int x, int y);

    // Terrain generation
    void generateHeightmap(int seed);
    void generateMoisture(int seed);
    void calculateNormals();
    void assignBiomes();

public:
    TerrainManager(int w, int d, float size = 5.0f);

    // Initialization
    void generate(int seed = 12345);
    void initializeBiomes();

    // Queries
    float getHeight(float x, float z) const;
    Vector3 getNormal(float x, float z) const;
    TerrainType getTerrainType(float x, float z) const;
    bool isWater(float x, float z) const;

    // Collision
    bool raycast(const Vector3& origin, const Vector3& direction, Vector3& hitPoint, float maxDistance = 1000.0f) const;

    // Rendering
    void render() const;
    void renderWireframe() const;

    // Getters
    int getWidth() const { return width; }
    int getDepth() const { return depth; }
    float getTileSize() const { return tileSize; }

    // Utilities
    void smooth(int iterations = 1);
    void addNoise(float amplitude);
};

#endif // TERRAIN_MANAGER_H
