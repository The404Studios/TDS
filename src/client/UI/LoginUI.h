#pragma once
#include "../Game.h"
namespace TDS {
class LoginUI {
public:
    LoginUI(Game* game);
    ~LoginUI();
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);
private:
    Game* game;
};
}
