#pragma once
#include "raylib.h"

namespace TDS {
class Game;

class Renderer {
public:
    Renderer(Game* game);
    ~Renderer();
    void render();
private:
    Game* game;
};
}
