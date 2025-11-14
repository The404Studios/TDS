#include "MainMenu.h"
#include "raylib.h"
namespace TDS {
MainMenu::MainMenu(Game* game) : game(game) {}
MainMenu::~MainMenu() {}
void MainMenu::update(float dt) {}
void MainMenu::render() {
    DrawText("MainMenu - TODO", 400, 300, 20, WHITE);
}
void MainMenu::onStateChanged(GameState newState) {}
}
