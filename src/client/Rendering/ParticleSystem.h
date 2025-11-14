#pragma once
namespace TDS {
class Game;
class ParticleSystem {
public:
    ParticleSystem(Game* game);
    ~ParticleSystem();
private:
    Game* game;
};
}
