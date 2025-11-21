// Placeholder Model Generator
// Creates simple 3D models when custom models aren't available

#pragma once

#include "RaylibPlatform.h"  // Platform-aware raylib header
#include <string>

class PlaceholderModels {
public:
    // Generate a simple humanoid character model
    static Model generatePlayerModel() {
        // Create a simple capsule-like character
        Mesh bodyMesh = GenMeshCube(0.5f, 1.5f, 0.3f);
        Mesh headMesh = GenMeshSphere(0.25f, 16, 16);
        Mesh legMesh = GenMeshCube(0.2f, 0.7f, 0.2f);
        Mesh armMesh = GenMeshCube(0.15f, 0.6f, 0.15f);

        // For simplicity, just use the body mesh as the main model
        // In a real implementation, you'd combine meshes
        Model model = LoadModelFromMesh(bodyMesh);

        // Set up material
        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BLUE;

        return model;
    }

    // Generate a simple corpse model (lying down)
    static Model generateCorpseModel() {
        // Create a flat, elongated box to represent a lying body
        Mesh corpseMesh = GenMeshCube(0.5f, 0.3f, 1.5f);
        Model model = LoadModelFromMesh(corpseMesh);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = DARKGRAY;

        return model;
    }

    // Generate a simple weapon model
    static Model generateWeaponModel() {
        Mesh weaponMesh = GenMeshCube(0.1f, 0.1f, 0.8f);
        Model model = LoadModelFromMesh(weaponMesh);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = DARKBROWN;

        return model;
    }

    // Generate a loot box model
    static Model generateLootBoxModel() {
        Mesh boxMesh = GenMeshCube(0.5f, 0.5f, 0.5f);
        Model model = LoadModelFromMesh(boxMesh);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = GOLD;

        return model;
    }

    // Generate ground plane
    static Model generateGroundModel(float width, float depth) {
        Mesh groundMesh = GenMeshPlane(width, depth, 10, 10);
        Model model = LoadModelFromMesh(groundMesh);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = ColorFromNormalized({0.3f, 0.5f, 0.3f, 1.0f});

        return model;
    }

    // Generate simple building/structure
    static Model generateBuildingModel() {
        Mesh buildingMesh = GenMeshCube(5.0f, 3.0f, 5.0f);
        Model model = LoadModelFromMesh(buildingMesh);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = GRAY;

        return model;
    }

    // Generate extraction zone marker
    static Model generateExtractionMarker() {
        Mesh markerMesh = GenMeshCylinder(2.0f, 0.1f, 16);
        Model model = LoadModelFromMesh(markerMesh);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = GREEN;

        return model;
    }

    // Apply a texture to a model (if you want to add textures later)
    static void applyCheckerTexture(Model& model) {
        Image checkedImg = GenImageChecked(256, 256, 32, 32, WHITE, LIGHTGRAY);
        Texture2D texture = LoadTextureFromImage(checkedImg);
        UnloadImage(checkedImg);

        model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    }

    // Create a simple grid floor texture
    static Texture2D generateGridTexture(int width, int height, int gridSize, Color color1, Color color2) {
        Image gridImg = GenImageChecked(width, height, gridSize, gridSize, color1, color2);
        Texture2D texture = LoadTextureFromImage(gridImg);
        UnloadImage(gridImg);
        return texture;
    }
};

// Asset Manager for lazy loading and caching models
class AssetManager {
private:
    Model playerModel;
    Model corpseModel;
    Model weaponModel;
    Model lootBoxModel;
    Model groundModel;
    Model extractionMarker;

    bool playerModelLoaded;
    bool corpseModelLoaded;
    bool weaponModelLoaded;
    bool lootBoxModelLoaded;
    bool groundModelLoaded;
    bool extractionMarkerLoaded;

public:
    AssetManager()
        : playerModelLoaded(false)
        , corpseModelLoaded(false)
        , weaponModelLoaded(false)
        , lootBoxModelLoaded(false)
        , groundModelLoaded(false)
        , extractionMarkerLoaded(false)
    {
    }

    ~AssetManager() {
        unloadAll();
    }

    Model& getPlayerModel(bool useCustom = true) {
        if (!playerModelLoaded) {
            if (useCustom && FileExists("resources/models/player.glb")) {
                playerModel = LoadModel("resources/models/player.glb");
                TraceLog(LOG_INFO, "Loaded custom player model");
            } else {
                playerModel = PlaceholderModels::generatePlayerModel();
                TraceLog(LOG_INFO, "Using placeholder player model");
            }
            playerModelLoaded = true;
        }
        return playerModel;
    }

    Model& getCorpseModel() {
        if (!corpseModelLoaded) {
            if (FileExists("resources/models/corpse.glb")) {
                corpseModel = LoadModel("resources/models/corpse.glb");
                TraceLog(LOG_INFO, "Loaded custom corpse model");
            } else {
                corpseModel = PlaceholderModels::generateCorpseModel();
                TraceLog(LOG_INFO, "Using placeholder corpse model");
            }
            corpseModelLoaded = true;
        }
        return corpseModel;
    }

    Model& getWeaponModel() {
        if (!weaponModelLoaded) {
            if (FileExists("resources/models/weapon.glb")) {
                weaponModel = LoadModel("resources/models/weapon.glb");
                TraceLog(LOG_INFO, "Loaded custom weapon model");
            } else {
                weaponModel = PlaceholderModels::generateWeaponModel();
                TraceLog(LOG_INFO, "Using placeholder weapon model");
            }
            weaponModelLoaded = true;
        }
        return weaponModel;
    }

    Model& getLootBoxModel() {
        if (!lootBoxModelLoaded) {
            if (FileExists("resources/models/lootbox.glb")) {
                lootBoxModel = LoadModel("resources/models/lootbox.glb");
                TraceLog(LOG_INFO, "Loaded custom loot box model");
            } else {
                lootBoxModel = PlaceholderModels::generateLootBoxModel();
                TraceLog(LOG_INFO, "Using placeholder loot box model");
            }
            lootBoxModelLoaded = true;
        }
        return lootBoxModel;
    }

    Model& getGroundModel() {
        if (!groundModelLoaded) {
            groundModel = PlaceholderModels::generateGroundModel(200.0f, 200.0f);
            groundModelLoaded = true;
        }
        return groundModel;
    }

    Model& getExtractionMarker() {
        if (!extractionMarkerLoaded) {
            extractionMarker = PlaceholderModels::generateExtractionMarker();
            extractionMarkerLoaded = true;
        }
        return extractionMarker;
    }

    void unloadAll() {
        if (playerModelLoaded) {
            UnloadModel(playerModel);
            playerModelLoaded = false;
        }
        if (corpseModelLoaded) {
            UnloadModel(corpseModel);
            corpseModelLoaded = false;
        }
        if (weaponModelLoaded) {
            UnloadModel(weaponModel);
            weaponModelLoaded = false;
        }
        if (lootBoxModelLoaded) {
            UnloadModel(lootBoxModel);
            lootBoxModelLoaded = false;
        }
        if (groundModelLoaded) {
            UnloadModel(groundModel);
            groundModelLoaded = false;
        }
        if (extractionMarkerLoaded) {
            UnloadModel(extractionMarker);
            extractionMarkerLoaded = false;
        }
    }
};
