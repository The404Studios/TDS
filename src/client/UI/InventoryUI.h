#pragma once
#include "../Game.h"
namespace TDS {
class InventoryUI {
public:
    InventoryUI(Game* game);
    ~InventoryUI();
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);
private:
    Game* game;
};
}
