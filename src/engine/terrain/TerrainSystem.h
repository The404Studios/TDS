#pragma once

#include "../rendering/RenderEngine.h"
#include "../../common/DataStructures.h"
#include <vector>
#include <memory>
#include <string>

/**
 * TerrainChunk - A section of terrain for LOD and culling
 */
struct TerrainChunk {
    int chunkX, chunkZ;
    int width, height;
    Vec3 position;
    std::vector<float> heightData;

    // Rendering data
    uint32_t meshId;
    uint32_t collisionMeshId;
    int lodLevel;

    // Bounds for culling
    Vec3 minBounds;
    Vec3 maxBounds;

    TerrainChunk() : chunkX(0), chunkZ(0), width(0), height(0),
                    meshId(0), collisionMeshId(0), lodLevel(0) {}
};

/**
 * TerrainLayer - Texture layer for terrain splatting
 */
struct TerrainLayer {
    std::string texturePath;
    std::string normalMapPath;
    float minHeight;     // Minimum height for this layer
    float maxHeight;     // Maximum height for this layer
    float minSlope;      // Minimum slope (0-90 degrees)
    float maxSlope;      // Maximum slope (0-90 degrees)
    float tileScale;     // UV tiling scale

    TerrainLayer() : minHeight(0.0f), maxHeight(100.0f),
                    minSlope(0.0f), maxSlope(90.0f), tileScale(1.0f) {}
};

/**
 * TerrainConfig - Configuration for terrain generation
 */
struct TerrainConfig {
    // Dimensions
    int terrainWidth;     // Total width in world units
    int terrainHeight;    // Total height in world units
    int heightResolution; // Resolution of heightmap (power of 2)
    float maxElevation;   // Maximum terrain height

    // Chunking
    int chunkSize;        // Size of each chunk (power of 2)
    bool enableChunking;

    // LOD settings
    bool enableLOD;
    int numLODLevels;
    std::vector<float> lodDistances;  // Distance thresholds for each LOD

    // Generation
    enum class GenerationType {
        FLAT,
        PERLIN_NOISE,
        HEIGHTMAP_IMAGE,
        CUSTOM
    };
    GenerationType generationType;

    // Perlin noise settings (for procedural generation)
    float perlinScale;
    float perlinPersistence;
    float perlinLacunarity;
    int perlinOctaves;
    uint32_t seed;

    // Heightmap image
    std::string heightmapPath;

    // Texture layers
    std::vector<TerrainLayer> layers;

    TerrainConfig()
        : terrainWidth(1000), terrainHeight(1000), heightResolution(512),
          maxElevation(100.0f), chunkSize(64), enableChunking(true),
          enableLOD(true), numLODLevels(3),
          generationType(GenerationType::PERLIN_NOISE),
          perlinScale(100.0f), perlinPersistence(0.5f),
          perlinLacunarity(2.0f), perlinOctaves(4), seed(12345) {

        // Default LOD distances
        lodDistances = {100.0f, 300.0f, 600.0f};
    }
};

/**
 * TerrainSystem - Manages terrain generation, rendering, and collision
 */
class TerrainSystem {
public:
    TerrainSystem();
    ~TerrainSystem();

    // Initialization
    bool initialize(const TerrainConfig& config);
    void shutdown();

    // Generation
    void generateTerrain();
    void generateFromHeightmap(const std::string& imagePath);
    void generateFromPerlinNoise();
    void generateFlat(float height = 0.0f);

    // Height queries
    float getHeightAt(float worldX, float worldZ) const;
    Vec3 getNormalAt(float worldX, float worldZ) const;
    float getSlopeAt(float worldX, float worldZ) const;

    // Chunk management
    void updateChunks(const Vec3& cameraPosition);
    void loadChunk(int chunkX, int chunkZ);
    void unloadChunk(int chunkX, int chunkZ);

    // LOD management
    void updateLOD(const Vec3& cameraPosition);
    int calculateLODLevel(const Vec3& chunkCenter, const Vec3& cameraPos) const;

    // Rendering
    void render(RenderEngine* renderer, const Vec3& cameraPosition);

    // Collision
    bool raycast(const Vec3& origin, const Vec3& direction, Vec3& hitPoint, float maxDistance = 1000.0f) const;

    // Modification
    void setHeightAt(int x, int z, float height);
    void raiseTerrainAt(float worldX, float worldZ, float radius, float amount);
    void smoothTerrain(float worldX, float worldZ, float radius);

    // Texture layers
    void addTextureLayer(const TerrainLayer& layer);
    void clearTextureLayers();

    // Utilities
    void saveHeightmap(const std::string& path) const;
    void loadHeightmap(const std::string& path);

    // Getters
    const TerrainConfig& getConfig() const { return config; }
    int getNumChunks() const { return static_cast<int>(chunks.size()); }
    int getActiveChunks() const;

private:
    TerrainConfig config;
    std::vector<float> heightData;  // Full heightmap
    std::vector<std::unique_ptr<TerrainChunk>> chunks;

    // Mesh generation
    void generateChunkMesh(TerrainChunk* chunk, int lodLevel);
    void generateChunkCollisionMesh(TerrainChunk* chunk);

    // Heightmap utilities
    void worldToHeightmap(float worldX, float worldZ, int& hmX, int& hmZ) const;
    float getHeightAtHeightmap(int hmX, int hmZ) const;
    void calculateNormal(int x, int z, Vec3& normal) const;

    // Perlin noise
    float perlinNoise(float x, float y) const;
    float perlinOctave(float x, float y, int octaves, float persistence) const;

    // Chunk utilities
    TerrainChunk* getChunk(int chunkX, int chunkZ);
    void createChunks();
    bool isChunkVisible(const TerrainChunk* chunk, const Vec3& cameraPosition) const;
};
