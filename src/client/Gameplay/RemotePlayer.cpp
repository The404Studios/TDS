#include "RemotePlayer.h"
#include "../Game.h"
namespace TDS {
RemotePlayer::RemotePlayer(Game* game) : game(game) {}
RemotePlayer::~RemotePlayer() {}
void RemotePlayer::update(float dt) {}
}
