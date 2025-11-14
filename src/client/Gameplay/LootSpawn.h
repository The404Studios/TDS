#pragma once
namespace TDS {
class Game;
class LootSpawn {
public:
    LootSpawn(Game* game);
    ~LootSpawn();
    void update(float dt);
private:
    Game* game;
};
}
