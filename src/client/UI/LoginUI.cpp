#include "LoginUI.h"
#include "raylib.h"
namespace TDS {
LoginUI::LoginUI(Game* game) : game(game) {}
LoginUI::~LoginUI() {}
void LoginUI::update(float dt) {}
void LoginUI::render() {
    DrawText("LoginUI - TODO", 400, 300, 20, WHITE);
}
void LoginUI::onStateChanged(GameState newState) {}
}
