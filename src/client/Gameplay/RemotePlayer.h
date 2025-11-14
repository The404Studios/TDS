#pragma once
namespace TDS {
class Game;
class RemotePlayer {
public:
    RemotePlayer(Game* game);
    ~RemotePlayer();
    void update(float dt);
private:
    Game* game;
};
}
