#pragma once
#include "../Game.h"
namespace TDS {
class UIManager {
public:
    UIManager(Game* game);
    ~UIManager();
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);
private:
    Game* game;
};
}
