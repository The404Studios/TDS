#include "InventoryUI.h"
#include "raylib.h"
namespace TDS {
InventoryUI::InventoryUI(Game* game) : game(game) {}
InventoryUI::~InventoryUI() {}
void InventoryUI::update(float dt) {}
void InventoryUI::render() {
    DrawText("InventoryUI - TODO", 400, 300, 20, WHITE);
}
void InventoryUI::onStateChanged(GameState newState) {}
}
