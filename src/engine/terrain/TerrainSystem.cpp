#include "TerrainSystem.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>

TerrainSystem::TerrainSystem() {
}

TerrainSystem::~TerrainSystem() {
    shutdown();
}

bool TerrainSystem::initialize(const TerrainConfig& cfg) {
    config = cfg;

    // Allocate heightmap
    int totalSize = config.heightResolution * config.heightResolution;
    heightData.resize(totalSize, 0.0f);

    std::cout << "[TerrainSystem] Initialized " << config.heightResolution << "x"
              << config.heightResolution << " terrain" << std::endl;

    return true;
}

void TerrainSystem::shutdown() {
    chunks.clear();
    heightData.clear();
    std::cout << "[TerrainSystem] Shutdown complete" << std::endl;
}

void TerrainSystem::generateTerrain() {
    switch (config.generationType) {
        case TerrainConfig::GenerationType::FLAT:
            generateFlat(0.0f);
            break;
        case TerrainConfig::GenerationType::PERLIN_NOISE:
            generateFromPerlinNoise();
            break;
        case TerrainConfig::GenerationType::HEIGHTMAP_IMAGE:
            if (!config.heightmapPath.empty()) {
                generateFromHeightmap(config.heightmapPath);
            }
            break;
        case TerrainConfig::GenerationType::CUSTOM:
            // User will set heights manually
            break;
    }

    // Create chunks after generation
    if (config.enableChunking) {
        createChunks();
    }

    std::cout << "[TerrainSystem] Terrain generation complete" << std::endl;
}

void TerrainSystem::generateFlat(float height) {
    for (size_t i = 0; i < heightData.size(); i++) {
        heightData[i] = height;
    }
    std::cout << "[TerrainSystem] Generated flat terrain at height " << height << std::endl;
}

void TerrainSystem::generateFromPerlinNoise() {
    for (int z = 0; z < config.heightResolution; z++) {
        for (int x = 0; x < config.heightResolution; x++) {
            float nx = (float)x / config.heightResolution;
            float nz = (float)z / config.heightResolution;

            float elevation = perlinOctave(
                nx * config.perlinScale,
                nz * config.perlinScale,
                config.perlinOctaves,
                config.perlinPersistence
            );

            // Normalize to [0, 1] and scale by max elevation
            elevation = (elevation + 1.0f) * 0.5f;  // [-1, 1] -> [0, 1]
            elevation *= config.maxElevation;

            int index = z * config.heightResolution + x;
            heightData[index] = elevation;
        }
    }

    std::cout << "[TerrainSystem] Generated Perlin noise terrain with "
              << config.perlinOctaves << " octaves" << std::endl;
}

void TerrainSystem::generateFromHeightmap(const std::string& imagePath) {
    // TODO: Load image and convert to heightmap
    // For now, generate with Perlin as fallback
    std::cout << "[TerrainSystem] Heightmap loading not yet implemented, using Perlin noise" << std::endl;
    generateFromPerlinNoise();
}

float TerrainSystem::getHeightAt(float worldX, float worldZ) const {
    int hmX, hmZ;
    worldToHeightmap(worldX, worldZ, hmX, hmZ);
    return getHeightAtHeightmap(hmX, hmZ);
}

Vec3 TerrainSystem::getNormalAt(float worldX, float worldZ) const {
    int hmX, hmZ;
    worldToHeightmap(worldX, worldZ, hmX, hmZ);

    Vec3 normal;
    calculateNormal(hmX, hmZ, normal);
    return normal;
}

float TerrainSystem::getSlopeAt(float worldX, float worldZ) const {
    Vec3 normal = getNormalAt(worldX, worldZ);
    Vec3 up(0, 1, 0);

    // Calculate angle between normal and up vector
    float dot = normal.x * up.x + normal.y * up.y + normal.z * up.z;
    float angle = std::acos(std::clamp(dot, -1.0f, 1.0f));

    // Convert to degrees
    return angle * 180.0f / 3.14159265f;
}

void TerrainSystem::updateChunks(const Vec3& cameraPosition) {
    // Update LOD based on camera distance
    if (config.enableLOD) {
        updateLOD(cameraPosition);
    }

    // TODO: Load/unload chunks based on distance
}

void TerrainSystem::loadChunk(int chunkX, int chunkZ) {
    // Check if chunk already loaded
    if (getChunk(chunkX, chunkZ)) {
        return;
    }

    // Create new chunk
    auto chunk = std::make_unique<TerrainChunk>();
    chunk->chunkX = chunkX;
    chunk->chunkZ = chunkZ;
    chunk->width = config.chunkSize;
    chunk->height = config.chunkSize;

    // Set position
    chunk->position.x = chunkX * config.chunkSize;
    chunk->position.y = 0;
    chunk->position.z = chunkZ * config.chunkSize;

    // Extract height data for this chunk
    chunk->heightData.resize(config.chunkSize * config.chunkSize);
    for (int z = 0; z < config.chunkSize; z++) {
        for (int x = 0; x < config.chunkSize; x++) {
            int worldX = chunkX * config.chunkSize + x;
            int worldZ = chunkZ * config.chunkSize + z;

            if (worldX < config.heightResolution && worldZ < config.heightResolution) {
                int hmIndex = worldZ * config.heightResolution + worldX;
                int chunkIndex = z * config.chunkSize + x;
                chunk->heightData[chunkIndex] = heightData[hmIndex];
            }
        }
    }

    // Generate mesh
    generateChunkMesh(chunk.get(), 0);
    generateChunkCollisionMesh(chunk.get());

    // Calculate bounds
    float minHeight = *std::min_element(chunk->heightData.begin(), chunk->heightData.end());
    float maxHeight = *std::max_element(chunk->heightData.begin(), chunk->heightData.end());

    chunk->minBounds = Vec3(chunk->position.x, minHeight, chunk->position.z);
    chunk->maxBounds = Vec3(
        chunk->position.x + config.chunkSize,
        maxHeight,
        chunk->position.z + config.chunkSize
    );

    chunks.push_back(std::move(chunk));
    std::cout << "[TerrainSystem] Loaded chunk (" << chunkX << ", " << chunkZ << ")" << std::endl;
}

void TerrainSystem::unloadChunk(int chunkX, int chunkZ) {
    auto it = std::remove_if(chunks.begin(), chunks.end(),
        [chunkX, chunkZ](const std::unique_ptr<TerrainChunk>& chunk) {
            return chunk->chunkX == chunkX && chunk->chunkZ == chunkZ;
        });

    if (it != chunks.end()) {
        chunks.erase(it, chunks.end());
        std::cout << "[TerrainSystem] Unloaded chunk (" << chunkX << ", " << chunkZ << ")" << std::endl;
    }
}

void TerrainSystem::updateLOD(const Vec3& cameraPosition) {
    for (auto& chunk : chunks) {
        Vec3 chunkCenter(
            chunk->position.x + config.chunkSize * 0.5f,
            chunk->position.y,
            chunk->position.z + config.chunkSize * 0.5f
        );

        int newLOD = calculateLODLevel(chunkCenter, cameraPosition);

        if (newLOD != chunk->lodLevel) {
            chunk->lodLevel = newLOD;
            generateChunkMesh(chunk.get(), newLOD);
        }
    }
}

int TerrainSystem::calculateLODLevel(const Vec3& chunkCenter, const Vec3& cameraPos) const {
    // Calculate distance from camera to chunk center
    float dx = chunkCenter.x - cameraPos.x;
    float dz = chunkCenter.z - cameraPos.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    // Determine LOD level based on distance
    for (int i = 0; i < config.numLODLevels; i++) {
        if (distance < config.lodDistances[i]) {
            return i;
        }
    }

    return config.numLODLevels - 1;  // Furthest LOD
}

void TerrainSystem::render(RenderEngine* renderer, const Vec3& cameraPosition) {
    if (!renderer) return;

    // Render each visible chunk
    for (const auto& chunk : chunks) {
        if (isChunkVisible(chunk.get(), cameraPosition)) {
            // TODO: Submit chunk mesh to renderer
            // renderer->renderMesh(chunk->meshId, chunk->position);
        }
    }
}

bool TerrainSystem::raycast(const Vec3& origin, const Vec3& direction, Vec3& hitPoint, float maxDistance) const {
    // Simple raycast - step along ray and check height
    float stepSize = 1.0f;
    Vec3 current = origin;
    Vec3 step = direction;

    // Normalize direction
    float length = std::sqrt(step.x * step.x + step.y * step.y + step.z * step.z);
    if (length > 0.0f) {
        step.x /= length;
        step.y /= length;
        step.z /= length;
    }

    step.x *= stepSize;
    step.y *= stepSize;
    step.z *= stepSize;

    for (float distance = 0; distance < maxDistance; distance += stepSize) {
        float terrainHeight = getHeightAt(current.x, current.z);

        if (current.y <= terrainHeight) {
            hitPoint = current;
            hitPoint.y = terrainHeight;
            return true;
        }

        current.x += step.x;
        current.y += step.y;
        current.z += step.z;
    }

    return false;
}

void TerrainSystem::setHeightAt(int x, int z, float height) {
    if (x >= 0 && x < config.heightResolution && z >= 0 && z < config.heightResolution) {
        int index = z * config.heightResolution + x;
        heightData[index] = height;
    }
}

void TerrainSystem::raiseTerrainAt(float worldX, float worldZ, float radius, float amount) {
    int centerX, centerZ;
    worldToHeightmap(worldX, worldZ, centerX, centerZ);

    int radiusInCells = static_cast<int>(radius);

    for (int z = centerZ - radiusInCells; z <= centerZ + radiusInCells; z++) {
        for (int x = centerX - radiusInCells; x <= centerX + radiusInCells; x++) {
            if (x < 0 || x >= config.heightResolution || z < 0 || z >= config.heightResolution) {
                continue;
            }

            // Calculate distance from center
            float dx = (float)(x - centerX);
            float dz = (float)(z - centerZ);
            float distance = std::sqrt(dx * dx + dz * dz);

            if (distance <= radius) {
                // Apply falloff (linear)
                float falloff = 1.0f - (distance / radius);
                int index = z * config.heightResolution + x;
                heightData[index] += amount * falloff;
            }
        }
    }
}

void TerrainSystem::smoothTerrain(float worldX, float worldZ, float radius) {
    int centerX, centerZ;
    worldToHeightmap(worldX, worldZ, centerX, centerZ);

    int radiusInCells = static_cast<int>(radius);

    for (int z = centerZ - radiusInCells; z <= centerZ + radiusInCells; z++) {
        for (int x = centerX - radiusInCells; x <= centerX + radiusInCells; x++) {
            if (x < 1 || x >= config.heightResolution - 1 || z < 1 || z >= config.heightResolution - 1) {
                continue;
            }

            // Average with neighbors
            float sum = 0.0f;
            int count = 0;

            for (int oz = -1; oz <= 1; oz++) {
                for (int ox = -1; ox <= 1; ox++) {
                    int index = (z + oz) * config.heightResolution + (x + ox);
                    sum += heightData[index];
                    count++;
                }
            }

            int index = z * config.heightResolution + x;
            heightData[index] = sum / count;
        }
    }
}

void TerrainSystem::addTextureLayer(const TerrainLayer& layer) {
    config.layers.push_back(layer);
}

void TerrainSystem::clearTextureLayers() {
    config.layers.clear();
}

void TerrainSystem::saveHeightmap(const std::string& path) const {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[TerrainSystem] Failed to save heightmap to " << path << std::endl;
        return;
    }

    // Write header
    int width = config.heightResolution;
    int height = config.heightResolution;
    file.write(reinterpret_cast<const char*>(&width), sizeof(int));
    file.write(reinterpret_cast<const char*>(&height), sizeof(int));

    // Write height data
    file.write(reinterpret_cast<const char*>(heightData.data()), heightData.size() * sizeof(float));

    file.close();
    std::cout << "[TerrainSystem] Saved heightmap to " << path << std::endl;
}

void TerrainSystem::loadHeightmap(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[TerrainSystem] Failed to load heightmap from " << path << std::endl;
        return;
    }

    // Read header
    int width, height;
    file.read(reinterpret_cast<char*>(&width), sizeof(int));
    file.read(reinterpret_cast<char*>(&height), sizeof(int));

    if (width != config.heightResolution || height != config.heightResolution) {
        std::cerr << "[TerrainSystem] Heightmap size mismatch" << std::endl;
        file.close();
        return;
    }

    // Read height data
    file.read(reinterpret_cast<char*>(heightData.data()), heightData.size() * sizeof(float));

    file.close();
    std::cout << "[TerrainSystem] Loaded heightmap from " << path << std::endl;

    // Recreate chunks
    if (config.enableChunking) {
        chunks.clear();
        createChunks();
    }
}

int TerrainSystem::getActiveChunks() const {
    return static_cast<int>(chunks.size());
}

// Private methods

void TerrainSystem::generateChunkMesh(TerrainChunk* chunk, int lodLevel) {
    // TODO: Generate vertex and index buffers for chunk
    // This would create a mesh based on the heightData with appropriate LOD

    // For now, just assign a placeholder mesh ID
    chunk->meshId = (chunk->chunkX * 1000 + chunk->chunkZ) * 10 + lodLevel;
    chunk->lodLevel = lodLevel;
}

void TerrainSystem::generateChunkCollisionMesh(TerrainChunk* chunk) {
    // TODO: Generate simplified collision mesh
    chunk->collisionMeshId = chunk->meshId + 1;
}

void TerrainSystem::worldToHeightmap(float worldX, float worldZ, int& hmX, int& hmZ) const {
    // Convert world coordinates to heightmap coordinates
    float scaleX = (float)config.heightResolution / (float)config.terrainWidth;
    float scaleZ = (float)config.heightResolution / (float)config.terrainHeight;

    hmX = static_cast<int>(worldX * scaleX);
    hmZ = static_cast<int>(worldZ * scaleZ);

    // Clamp to valid range
    hmX = std::clamp(hmX, 0, config.heightResolution - 1);
    hmZ = std::clamp(hmZ, 0, config.heightResolution - 1);
}

float TerrainSystem::getHeightAtHeightmap(int hmX, int hmZ) const {
    if (hmX < 0 || hmX >= config.heightResolution || hmZ < 0 || hmZ >= config.heightResolution) {
        return 0.0f;
    }

    int index = hmZ * config.heightResolution + hmX;
    return heightData[index];
}

void TerrainSystem::calculateNormal(int x, int z, Vec3& normal) const {
    // Get heights of neighbors
    float hL = getHeightAtHeightmap(x - 1, z);
    float hR = getHeightAtHeightmap(x + 1, z);
    float hD = getHeightAtHeightmap(x, z - 1);
    float hU = getHeightAtHeightmap(x, z + 1);

    // Calculate normal using cross product
    Vec3 tangentX(2.0f, hR - hL, 0.0f);
    Vec3 tangentZ(0.0f, hU - hD, 2.0f);

    // Cross product
    normal.x = tangentX.y * tangentZ.z - tangentX.z * tangentZ.y;
    normal.y = tangentX.z * tangentZ.x - tangentX.x * tangentZ.z;
    normal.z = tangentX.x * tangentZ.y - tangentX.y * tangentZ.x;

    // Normalize
    float length = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
    if (length > 0.0f) {
        normal.x /= length;
        normal.y /= length;
        normal.z /= length;
    }
}

float TerrainSystem::perlinNoise(float x, float y) const {
    // Simple Perlin noise implementation
    // This is a placeholder - a real implementation would use proper Perlin/Simplex noise

    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;

    float xf = x - std::floor(x);
    float yf = y - std::floor(y);

    // Fade curves
    float u = xf * xf * (3.0f - 2.0f * xf);
    float v = yf * yf * (3.0f - 2.0f * yf);

    // Hash coordinates
    int aa = (xi + yi * 57 + config.seed) % 256;
    int ab = (xi + (yi + 1) * 57 + config.seed) % 256;
    int ba = ((xi + 1) + yi * 57 + config.seed) % 256;
    int bb = ((xi + 1) + (yi + 1) * 57 + config.seed) % 256;

    // Interpolate
    float x1 = std::lerp((float)(aa % 100) / 50.0f - 1.0f, (float)(ba % 100) / 50.0f - 1.0f, u);
    float x2 = std::lerp((float)(ab % 100) / 50.0f - 1.0f, (float)(bb % 100) / 50.0f - 1.0f, u);

    return std::lerp(x1, x2, v);
}

float TerrainSystem::perlinOctave(float x, float y, int octaves, float persistence) const {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; i++) {
        total += perlinNoise(x * frequency, y * frequency) * amplitude;

        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= config.perlinLacunarity;
    }

    return total / maxValue;
}

TerrainChunk* TerrainSystem::getChunk(int chunkX, int chunkZ) {
    for (auto& chunk : chunks) {
        if (chunk->chunkX == chunkX && chunk->chunkZ == chunkZ) {
            return chunk.get();
        }
    }
    return nullptr;
}

void TerrainSystem::createChunks() {
    int numChunksX = config.heightResolution / config.chunkSize;
    int numChunksZ = config.heightResolution / config.chunkSize;

    std::cout << "[TerrainSystem] Creating " << (numChunksX * numChunksZ) << " chunks..." << std::endl;

    for (int cz = 0; cz < numChunksZ; cz++) {
        for (int cx = 0; cx < numChunksX; cx++) {
            loadChunk(cx, cz);
        }
    }
}

bool TerrainSystem::isChunkVisible(const TerrainChunk* chunk, const Vec3& cameraPosition) const {
    // Simple distance-based culling
    Vec3 chunkCenter(
        chunk->position.x + config.chunkSize * 0.5f,
        chunk->position.y,
        chunk->position.z + config.chunkSize * 0.5f
    );

    float dx = chunkCenter.x - cameraPosition.x;
    float dz = chunkCenter.z - cameraPosition.z;
    float distance = std::sqrt(dx * dx + dz * dz);

    // Visible if within max LOD distance
    return distance < config.lodDistances[config.numLODLevels - 1] * 1.5f;
}
