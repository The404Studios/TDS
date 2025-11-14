#include "Player.h"
#include "../Game.h"
namespace TDS {
Player::Player(Game* game) : game(game) {}
Player::~Player() {}
void Player::update(float dt) {}
}
