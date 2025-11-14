#pragma once
namespace TDS {
class Game;
class ModelManager {
public:
    ModelManager(Game* game);
    ~ModelManager();
private:
    Game* game;
};
}
