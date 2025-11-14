#include "HUD.h"
#include "raylib.h"
namespace TDS {
HUD::HUD(Game* game) : game(game) {}
HUD::~HUD() {}
void HUD::update(float dt) {}
void HUD::render() {
    DrawText("HUD - TODO", 400, 300, 20, WHITE);
}
void HUD::onStateChanged(GameState newState) {}
}
