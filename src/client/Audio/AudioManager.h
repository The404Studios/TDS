#pragma once
namespace TDS {
class Game;
class AudioManager {
public:
    AudioManager(Game* game);
    ~AudioManager();
private:
    Game* game;
};
}
