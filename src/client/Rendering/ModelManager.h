#pragma once

#include "raylib.h"
#include <map>
#include <string>

namespace TDS {

class Game;

class ModelManager {
public:
    ModelManager(Game* game);
    ~ModelManager();

    void loadDefaultModels();
    void loadWeaponModels();
    Model* getModel(const std::string& name);
    void unloadAll();

private:
    Game* game;
    std::map<std::string, Model> models;
};

} // namespace TDS
