#include "ModelManager.h"
#include "../Game.h"
#include "raylib.h"
#include <iostream>

namespace TDS {

ModelManager::ModelManager(Game* game) : game(game) {
    loadDefaultModels();
}

ModelManager::~ModelManager() {
    unloadAll();
}

void ModelManager::loadDefaultModels() {
    std::cout << "[ModelManager] Loading default models..." << std::endl;

    // Load cube placeholder
    if (FileExists("assets/models/cube.obj")) {
        Model cubeModel = LoadModel("assets/models/cube.obj");
        models["cube"] = cubeModel;
        std::cout << "[ModelManager] Loaded: cube.obj" << std::endl;
    } else {
        // Generate procedural cube
        Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
        Model cubeModel = LoadModelFromMesh(cubeMesh);
        models["cube"] = cubeModel;
        std::cout << "[ModelManager] Generated: procedural cube" << std::endl;
    }

    // Load floor
    if (FileExists("assets/models/floor.obj")) {
        Model floorModel = LoadModel("assets/models/floor.obj");
        models["floor"] = floorModel;
        std::cout << "[ModelManager] Loaded: floor.obj" << std::endl;
    } else {
        // Generate procedural plane
        Mesh planeMesh = GenMeshPlane(20.0f, 20.0f, 10, 10);
        Model planeModel = LoadModelFromMesh(planeMesh);
        models["floor"] = planeModel;
        std::cout << "[ModelManager] Generated: procedural floor" << std::endl;
    }

    // Load weapons if available
    loadWeaponModels();
}

void ModelManager::loadWeaponModels() {
    const char* weaponFiles[] = {
        "assets/models/weapons/ak74.obj",
        "assets/models/weapons/m4a1.obj",
        "assets/models/weapons/pistol.obj"
    };

    for (const char* file : weaponFiles) {
        if (FileExists(file)) {
            Model weaponModel = LoadModel(file);
            
            // Extract filename without path/extension
            std::string path(file);
            size_t lastSlash = path.find_last_of("/\\");
            size_t lastDot = path.find_last_of(".");
            std::string name = path.substr(lastSlash + 1, lastDot - lastSlash - 1);

            models[name] = weaponModel;
            std::cout << "[ModelManager] Loaded weapon: " << name << std::endl;
        }
    }

    // If no weapons loaded, use cube as placeholder
    if (models.find("weapon") == models.end()) {
        Mesh weaponMesh = GenMeshCube(0.1f, 0.1f, 0.5f);
        Model weaponModel = LoadModelFromMesh(weaponMesh);
        models["weapon"] = weaponModel;
        std::cout << "[ModelManager] Generated: procedural weapon placeholder" << std::endl;
    }
}

Model* ModelManager::getModel(const std::string& name) {
    auto it = models.find(name);
    if (it != models.end()) {
        return &it->second;
    }

    // Return cube as fallback
    return &models["cube"];
}

void ModelManager::unloadAll() {
    for (auto& pair : models) {
        UnloadModel(pair.second);
    }
    models.clear();
    std::cout << "[ModelManager] All models unloaded" << std::endl;
}

} // namespace TDS
