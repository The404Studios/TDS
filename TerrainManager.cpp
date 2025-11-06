#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <GL/gl.h>
#include "TerrainManager.h"
#include <cstdlib>
#include <ctime>

TerrainManager::TerrainManager(int w, int d, float size)
    : width(w), depth(d), tileSize(size) {
    tiles.resize(width);
    for (int x = 0; x < width; ++x) {
        tiles[x].resize(depth);
    }
    initializeBiomes();
}

void TerrainManager::initializeBiomes() {
    // Define biomes based on height and moisture
    biomes.push_back(Biome(TerrainType::WATER, -1.0f, 0.3f, 0.0f, 1.0f, 0.2f, 0.4f, 0.8f));
    biomes.push_back(Biome(TerrainType::SAND, 0.3f, 0.4f, 0.0f, 0.3f, 0.9f, 0.9f, 0.6f));
    biomes.push_back(Biome(TerrainType::GRASS, 0.4f, 0.7f, 0.3f, 0.7f, 0.3f, 0.7f, 0.3f));
    biomes.push_back(Biome(TerrainType::DIRT, 0.4f, 0.6f, 0.0f, 0.3f, 0.6f, 0.5f, 0.3f));
    biomes.push_back(Biome(TerrainType::STONE, 0.7f, 0.9f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f));
    biomes.push_back(Biome(TerrainType::SNOW, 0.8f, 1.0f, 0.5f, 1.0f, 0.95f, 0.95f, 0.95f));
}

float TerrainManager::noise2D(int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}

float TerrainManager::interpolate(float a, float b, float t) {
    float ft = t * 3.1415927f;
    float f = (1.0f - cosf(ft)) * 0.5f;
    return a * (1.0f - f) + b * f;
}

float TerrainManager::perlinNoise(float x, float y, int octaves, float persistence) {
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        int ix = static_cast<int>(x * frequency);
        int iy = static_cast<int>(y * frequency);
        float fx = x * frequency - ix;
        float fy = y * frequency - iy;

        float v1 = noise2D(ix, iy);
        float v2 = noise2D(ix + 1, iy);
        float v3 = noise2D(ix, iy + 1);
        float v4 = noise2D(ix + 1, iy + 1);

        float i1 = interpolate(v1, v2, fx);
        float i2 = interpolate(v3, v4, fx);
        float value = interpolate(i1, i2, fy);

        total += value * amplitude;
        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue;
}

void TerrainManager::generateHeightmap(int seed) {
    srand(seed);

    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            float nx = static_cast<float>(x) / width;
            float nz = static_cast<float>(z) / depth;

            // Multiple octaves for detail
            float height = perlinNoise(nx * 4.0f + seed, nz * 4.0f + seed, 6, 0.5f);

            // Normalize to 0-1 range
            height = (height + 1.0f) * 0.5f;

            // Apply some post-processing
            height = powf(height, 1.2f); // Make valleys deeper

            tiles[x][z].height = height * 50.0f; // Scale to actual height
        }
    }
}

void TerrainManager::generateMoisture(int seed) {
    srand(seed + 1000);

    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            float nx = static_cast<float>(x) / width;
            float nz = static_cast<float>(z) / depth;

            float moisture = perlinNoise(nx * 2.0f + seed, nz * 2.0f + seed, 4, 0.6f);
            moisture = (moisture + 1.0f) * 0.5f;

            tiles[x][z].moisture = moisture;
        }
    }
}

void TerrainManager::calculateNormals() {
    for (int x = 1; x < width - 1; ++x) {
        for (int z = 1; z < depth - 1; ++z) {
            float hL = tiles[x - 1][z].height;
            float hR = tiles[x + 1][z].height;
            float hD = tiles[x][z - 1].height;
            float hU = tiles[x][z + 1].height;

            Vector3 normal;
            normal.x = (hL - hR) / (2.0f * tileSize);
            normal.y = (hD - hU) / (2.0f * tileSize);
            normal.z = 1.0f;

            // Normalize
            float length = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
            if (length > 0) {
                normal.x /= length;
                normal.y /= length;
                normal.z /= length;
            }

            tiles[x][z].normal = normal;
        }
    }
}

void TerrainManager::assignBiomes() {
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            float normalizedHeight = tiles[x][z].height / 50.0f;
            float moisture = tiles[x][z].moisture;

            // Find matching biome
            tiles[x][z].type = TerrainType::GRASS; // Default

            for (const auto& biome : biomes) {
                if (normalizedHeight >= biome.minHeight && normalizedHeight <= biome.maxHeight &&
                    moisture >= biome.minMoisture && moisture <= biome.maxMoisture) {
                    tiles[x][z].type = biome.type;
                    break;
                }
            }
        }
    }
}

void TerrainManager::generate(int seed) {
    generateHeightmap(seed);
    generateMoisture(seed);
    calculateNormals();
    assignBiomes();
}

float TerrainManager::getHeight(float x, float z) const {
    int ix = static_cast<int>(x / tileSize);
    int iz = static_cast<int>(z / tileSize);

    if (ix < 0 || ix >= width - 1 || iz < 0 || iz >= depth - 1) {
        return 0.0f;
    }

    // Bilinear interpolation
    float fx = (x / tileSize) - ix;
    float fz = (z / tileSize) - iz;

    float h00 = tiles[ix][iz].height;
    float h10 = tiles[ix + 1][iz].height;
    float h01 = tiles[ix][iz + 1].height;
    float h11 = tiles[ix + 1][iz + 1].height;

    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;

    return h0 * (1.0f - fz) + h1 * fz;
}

Vector3 TerrainManager::getNormal(float x, float z) const {
    int ix = static_cast<int>(x / tileSize);
    int iz = static_cast<int>(z / tileSize);

    if (ix < 0 || ix >= width || iz < 0 || iz >= depth) {
        return Vector3(0, 0, 1);
    }

    return tiles[ix][iz].normal;
}

TerrainType TerrainManager::getTerrainType(float x, float z) const {
    int ix = static_cast<int>(x / tileSize);
    int iz = static_cast<int>(z / tileSize);

    if (ix < 0 || ix >= width || iz < 0 || iz >= depth) {
        return TerrainType::GRASS;
    }

    return tiles[ix][iz].type;
}

bool TerrainManager::isWater(float x, float z) const {
    return getTerrainType(x, z) == TerrainType::WATER;
}

bool TerrainManager::raycast(const Vector3& origin, const Vector3& direction, Vector3& hitPoint, float maxDistance) const {
    float stepSize = 1.0f;
    Vector3 current = origin;

    for (float dist = 0; dist < maxDistance; dist += stepSize) {
        current = origin + direction * dist;
        float terrainHeight = getHeight(current.x, current.y);

        if (current.z <= terrainHeight) {
            hitPoint = Vector3(current.x, current.y, terrainHeight);
            return true;
        }
    }

    return false;
}

void TerrainManager::render() const {
    glEnable(GL_LIGHTING);

    for (int x = 0; x < width - 1; ++x) {
        glBegin(GL_TRIANGLE_STRIP);
        for (int z = 0; z < depth; ++z) {
            // Get biome color
            float r = 0.3f, g = 0.7f, b = 0.3f; // Default grass
            for (const auto& biome : biomes) {
                if (tiles[x][z].type == biome.type) {
                    r = biome.r;
                    g = biome.g;
                    b = biome.b;
                    break;
                }
            }

            glColor3f(r, g, b);
            glNormal3f(tiles[x][z].normal.x, tiles[x][z].normal.y, tiles[x][z].normal.z);
            glVertex3f(x * tileSize, z * tileSize, tiles[x][z].height);

            // Right tile
            for (const auto& biome : biomes) {
                if (tiles[x + 1][z].type == biome.type) {
                    r = biome.r;
                    g = biome.g;
                    b = biome.b;
                    break;
                }
            }

            glColor3f(r, g, b);
            glNormal3f(tiles[x + 1][z].normal.x, tiles[x + 1][z].normal.y, tiles[x + 1][z].normal.z);
            glVertex3f((x + 1) * tileSize, z * tileSize, tiles[x + 1][z].height);
        }
        glEnd();
    }
}

void TerrainManager::renderWireframe() const {
    glDisable(GL_LIGHTING);
    glColor3f(0.5f, 0.5f, 0.5f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    render();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_LIGHTING);
}

void TerrainManager::smooth(int iterations) {
    for (int iter = 0; iter < iterations; ++iter) {
        std::vector<std::vector<TerrainTile>> newTiles = tiles;

        for (int x = 1; x < width - 1; ++x) {
            for (int z = 1; z < depth - 1; ++z) {
                float avg = 0;
                avg += tiles[x - 1][z].height;
                avg += tiles[x + 1][z].height;
                avg += tiles[x][z - 1].height;
                avg += tiles[x][z + 1].height;
                avg += tiles[x][z].height * 4.0f;
                avg /= 8.0f;

                newTiles[x][z].height = avg;
            }
        }

        tiles = newTiles;
    }

    calculateNormals();
}

void TerrainManager::addNoise(float amplitude) {
    for (int x = 0; x < width; ++x) {
        for (int z = 0; z < depth; ++z) {
            tiles[x][z].height += ((rand() % 100) / 100.0f - 0.5f) * amplitude;
        }
    }

    calculateNormals();
}
