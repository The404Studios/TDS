#include "UIManager.h"
#include "raylib.h"
namespace TDS {
UIManager::UIManager(Game* game) : game(game) {}
UIManager::~UIManager() {}
void UIManager::update(float dt) {}
void UIManager::render() {
    DrawText("UIManager - TODO", 400, 300, 20, WHITE);
}
void UIManager::onStateChanged(GameState newState) {}
}
