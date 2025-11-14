#pragma once
namespace TDS {
class Game;
class Weapon {
public:
    Weapon(Game* game);
    ~Weapon();
    void update(float dt);
private:
    Game* game;
};
}
