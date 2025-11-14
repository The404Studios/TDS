#pragma once
#include "../Game.h"

namespace TDS {

class MainMenu {
public:
    MainMenu(Game* game);
    ~MainMenu();
    
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);
    
private:
    void handleSelection();
    
    Game* game;
    int selectedOption;
    float animProgress;
};

} // namespace TDS
