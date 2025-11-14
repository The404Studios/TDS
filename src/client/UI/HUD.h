#pragma once
#include "../Game.h"
namespace TDS {
class HUD {
public:
    HUD(Game* game);
    ~HUD();
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);
private:
    Game* game;
};
}
