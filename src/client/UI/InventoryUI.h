#pragma once
#include "../Game.h"
#include "common/Items.h"
#include <vector>

namespace TDS {

struct InventorySlot {
    uint16_t itemId;
    uint16_t quantity;
    int gridX;
    int gridY;
};

class InventoryUI {
public:
    InventoryUI(Game* game);
    ~InventoryUI();
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);

private:
    void renderInventoryGrid();
    void renderItemDetails();
    void handleItemInteraction();
    int getSlotAtPosition(int gridX, int gridY);

    Game* game;

    // Inventory grid (Tarkov-style)
    static constexpr int GRID_WIDTH = 10;
    static constexpr int GRID_HEIGHT = 8;
    static constexpr int CELL_SIZE = 60;

    std::vector<InventorySlot> items;

    int selectedSlot;
    int hoveredSlot;
    bool isDragging;
    InventorySlot draggedItem;
};

} // namespace TDS
