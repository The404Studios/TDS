#pragma once
namespace TDS {
class Game;
class Player {
public:
    Player(Game* game);
    ~Player();
    void update(float dt);
private:
    Game* game;
};
}
