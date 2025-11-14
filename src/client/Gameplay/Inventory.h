#pragma once
namespace TDS {
class Game;
class Inventory {
public:
    Inventory(Game* game);
    ~Inventory();
    void update(float dt);
private:
    Game* game;
};
}
