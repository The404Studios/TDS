#include "Weapon.h"
#include "../Game.h"
namespace TDS {
Weapon::Weapon(Game* game) : game(game) {}
Weapon::~Weapon() {}
void Weapon::update(float dt) {}
}
