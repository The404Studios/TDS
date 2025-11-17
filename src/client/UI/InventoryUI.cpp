#include "InventoryUI.h"
#include "UIManager.h"
#include "raylib.h"
#include <algorithm>

namespace TDS {

InventoryUI::InventoryUI(Game* game)
    : game(game)
    , selectedSlot(-1)
    , hoveredSlot(-1)
    , isDragging(false)
{
    // Add some test items to the inventory
    items.push_back({Items::AK74, 1, 0, 0});
    items.push_back({Items::M4A1, 1, 3, 0});
    items.push_back({Items::GLOCK17, 1, 0, 2});
    items.push_back({Items::IFAK, 2, 7, 0});
    items.push_back({Items::AMMO_545x39, 60, 8, 0});
}

InventoryUI::~InventoryUI() {}

void InventoryUI::update(float dt) {
    handleItemInteraction();
}

void InventoryUI::render() {
    auto theme = UIManager::getTheme();
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();

    ClearBackground(theme.background);

    // Title
    DrawText("INVENTORY", 40, 30, 32, theme.accent);
    DrawText("Press TAB to close", 40, 70, 16, theme.textDark);

    // Main inventory panel
    Rectangle inventoryPanel = {
        40,
        120,
        GRID_WIDTH * CELL_SIZE + 40.0f,
        GRID_HEIGHT * CELL_SIZE + 80.0f
    };
    UIManager::drawPanel(inventoryPanel, "PLAYER INVENTORY");

    // Render inventory grid
    renderInventoryGrid();

    // Item details panel (right side)
    Rectangle detailsPanel = {
        inventoryPanel.x + inventoryPanel.width + 20,
        120,
        400,
        500
    };
    UIManager::drawPanel(detailsPanel, "ITEM DETAILS");
    renderItemDetails();

    // Player stats panel (bottom right)
    Rectangle statsPanel = {
        detailsPanel.x,
        detailsPanel.y + detailsPanel.height + 20,
        400,
        200
    };
    UIManager::drawPanel(statsPanel, "PLAYER STATS");

    // Draw player stats
    float startY = statsPanel.y + 60;
    DrawText(TextFormat("Health: 100 / 100"), statsPanel.x + 20, startY, 18, theme.text);
    DrawText(TextFormat("Weight: 15.3 / 50.0 kg"), statsPanel.x + 20, startY + 30, 18, theme.text);
    DrawText(TextFormat("Money: 45,320₽"), statsPanel.x + 20, startY + 60, 18, theme.accent);
    DrawText(TextFormat("Level: 12"), statsPanel.x + 20, startY + 90, 18, theme.text);

    // Instructions
    const char* instructions = "Left Click: Select | Right Click: Use | Drag: Move Item";
    DrawText(instructions, 40, screenHeight - 40, 14, theme.textDark);
}

void InventoryUI::renderInventoryGrid() {
    auto theme = UIManager::getTheme();
    const int startX = 60;
    const int startY = 180;

    Vector2 mousePos = GetMousePosition();
    hoveredSlot = -1;

    // Draw grid cells
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            Rectangle cell = {
                (float)(startX + x * CELL_SIZE),
                (float)(startY + y * CELL_SIZE),
                (float)(CELL_SIZE - 2),
                (float)(CELL_SIZE - 2)
            };

            bool isHovered = CheckCollisionPointRec(mousePos, cell);

            // Draw cell background
            Color cellColor = theme.panelLight;
            if (isHovered) {
                cellColor = theme.panelDark;
                hoveredSlot = y * GRID_WIDTH + x;
            }

            DrawRectangleRec(cell, cellColor);
            DrawRectangleLinesEx(cell, 1, theme.textDark);
        }
    }

    // Draw items in grid
    for (size_t i = 0; i < items.size(); i++) {
        const auto& slot = items[i];
        const ItemDefinition* itemDef = ItemDatabase::getItem(slot.itemId);

        if (!itemDef) continue;

        Rectangle itemRect = {
            (float)(startX + slot.gridX * CELL_SIZE),
            (float)(startY + slot.gridY * CELL_SIZE),
            (float)(itemDef->gridWidth * CELL_SIZE - 2),
            (float)(itemDef->gridHeight * CELL_SIZE - 2)
        };

        bool isSelected = (int)i == selectedSlot;
        bool isHovered = CheckCollisionPointRec(mousePos, itemRect);

        // Draw item background
        Color itemBgColor = theme.panelDark;
        if (isSelected) {
            itemBgColor = Fade(theme.accent, 0.3f);
        } else if (isHovered) {
            itemBgColor = Fade(theme.accentHover, 0.2f);
        }

        DrawRectangleRec(itemRect, itemBgColor);

        // Draw item border based on rarity
        Color borderColor = theme.textDark;
        switch (itemDef->rarity) {
            case ItemRarity::COMMON: borderColor = GRAY; break;
            case ItemRarity::UNCOMMON: borderColor = GREEN; break;
            case ItemRarity::RARE: borderColor = BLUE; break;
            case ItemRarity::EPIC: borderColor = PURPLE; break;
            case ItemRarity::LEGENDARY: borderColor = GOLD; break;
        }
        DrawRectangleLinesEx(itemRect, 2, borderColor);

        // Draw item name (truncated)
        const char* itemName = itemDef->name.c_str();
        DrawText(itemName, itemRect.x + 5, itemRect.y + 5, 12, theme.text);

        // Draw quantity if stackable
        if (itemDef->stackable && slot.quantity > 1) {
            const char* qtyText = TextFormat("x%d", slot.quantity);
            int qtyWidth = MeasureText(qtyText, 16);
            DrawText(qtyText, itemRect.x + itemRect.width - qtyWidth - 5,
                    itemRect.y + itemRect.height - 20, 16, theme.accent);
        }

        // Draw item icon placeholder
        DrawRectangle(itemRect.x + 5, itemRect.y + 25, 30, 30, Fade(borderColor, 0.5f));
    }

    // Draw dragged item (if any)
    if (isDragging) {
        const ItemDefinition* itemDef = ItemDatabase::getItem(draggedItem.itemId);
        if (itemDef) {
            Rectangle dragRect = {
                mousePos.x - (CELL_SIZE / 2.0f),
                mousePos.y - (CELL_SIZE / 2.0f),
                (float)(itemDef->gridWidth * CELL_SIZE),
                (float)(itemDef->gridHeight * CELL_SIZE)
            };
            DrawRectangleRec(dragRect, Fade(theme.accent, 0.7f));
            DrawRectangleLinesEx(dragRect, 2, theme.accentHover);
            DrawText(itemDef->name.c_str(), dragRect.x + 5, dragRect.y + 5, 12, theme.text);
        }
    }
}

void InventoryUI::renderItemDetails() {
    auto theme = UIManager::getTheme();
    const int detailsX = 700;
    const int detailsY = 180;

    if (selectedSlot >= 0 && selectedSlot < (int)items.size()) {
        const auto& slot = items[selectedSlot];
        const ItemDefinition* itemDef = ItemDatabase::getItem(slot.itemId);

        if (itemDef) {
            // Item name
            DrawText(itemDef->name.c_str(), detailsX, detailsY, 24, theme.accent);

            // Category
            const char* category = "Unknown";
            switch (itemDef->category) {
                case ItemCategory::WEAPON: category = "Weapon"; break;
                case ItemCategory::AMMO: category = "Ammunition"; break;
                case ItemCategory::ARMOR: category = "Armor"; break;
                case ItemCategory::HELMET: category = "Helmet"; break;
                case ItemCategory::BACKPACK: category = "Backpack"; break;
                case ItemCategory::MEDICAL: category = "Medical"; break;
                case ItemCategory::FOOD: category = "Food"; break;
                case ItemCategory::VALUABLE: category = "Valuable"; break;
                case ItemCategory::MATERIAL: category = "Material"; break;
                case ItemCategory::KEY: category = "Key"; break;
                case ItemCategory::ATTACHMENT: category = "Attachment"; break;
            }
            DrawText(category, detailsX, detailsY + 35, 16, theme.textDark);

            // Rarity
            const char* rarity = "Common";
            Color rarityColor = GRAY;
            switch (itemDef->rarity) {
                case ItemRarity::UNCOMMON: rarity = "Uncommon"; rarityColor = GREEN; break;
                case ItemRarity::RARE: rarity = "Rare"; rarityColor = BLUE; break;
                case ItemRarity::EPIC: rarity = "Epic"; rarityColor = PURPLE; break;
                case ItemRarity::LEGENDARY: rarity = "Legendary"; rarityColor = GOLD; break;
                default: break;
            }
            DrawText(rarity, detailsX, detailsY + 60, 16, rarityColor);

            // Separator
            DrawLineEx(
                Vector2{(float)detailsX, (float)(detailsY + 90)},
                Vector2{(float)(detailsX + 360), (float)(detailsY + 90)},
                2, theme.panelLight
            );

            // Description
            DrawText("Description:", detailsX, detailsY + 110, 16, theme.text);
            DrawText(itemDef->description.c_str(), detailsX, detailsY + 135, 14, theme.textDark);

            // Stats
            DrawText("Stats:", detailsX, detailsY + 200, 16, theme.text);
            DrawText(TextFormat("Weight: %.1f kg", itemDef->weight / 1000.0f), detailsX, detailsY + 225, 14, theme.textDark);
            DrawText(TextFormat("Value: %d₽", itemDef->baseValue), detailsX, detailsY + 245, 14, theme.textDark);
            DrawText(TextFormat("Size: %dx%d", itemDef->gridWidth, itemDef->gridHeight), detailsX, detailsY + 265, 14, theme.textDark);

            if (itemDef->stackable) {
                DrawText(TextFormat("Stackable (max: %d)", itemDef->maxStack), detailsX, detailsY + 285, 14, theme.textDark);
            }

            // Action buttons
            Rectangle useBtn = {(float)detailsX, (float)(detailsY + 330), 170, 40};
            Rectangle dropBtn = {(float)(detailsX + 190), (float)(detailsY + 330), 170, 40};

            if (UIManager::drawButtonEx(useBtn, "USE", theme.accent, theme.accentHover)) {
                TraceLog(LOG_INFO, "Use item: %s", itemDef->name.c_str());
                // TODO: Implement item use logic
            }

            if (UIManager::drawButtonEx(dropBtn, "DROP", theme.danger, Fade(theme.danger, 0.8f))) {
                TraceLog(LOG_INFO, "Drop item: %s", itemDef->name.c_str());
                // TODO: Implement item drop logic
            }
        }
    } else {
        // No item selected
        DrawText("Select an item to view details", detailsX, detailsY + 100, 16, theme.textDark);
    }
}

void InventoryUI::handleItemInteraction() {
    Vector2 mousePos = GetMousePosition();

    // Left click to select
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        for (size_t i = 0; i < items.size(); i++) {
            const auto& slot = items[i];
            const ItemDefinition* itemDef = ItemDatabase::getItem(slot.itemId);
            if (!itemDef) continue;

            Rectangle itemRect = {
                (float)(60 + slot.gridX * CELL_SIZE),
                (float)(180 + slot.gridY * CELL_SIZE),
                (float)(itemDef->gridWidth * CELL_SIZE),
                (float)(itemDef->gridHeight * CELL_SIZE)
            };

            if (CheckCollisionPointRec(mousePos, itemRect)) {
                selectedSlot = (int)i;
                // Start dragging
                isDragging = true;
                draggedItem = slot;
                return;
            }
        }
        // Clicked empty space
        selectedSlot = -1;
    }

    // Stop dragging
    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isDragging) {
        isDragging = false;

        // Calculate grid position
        int gridX = ((int)mousePos.x - 60) / CELL_SIZE;
        int gridY = ((int)mousePos.y - 180) / CELL_SIZE;

        if (gridX >= 0 && gridX < GRID_WIDTH && gridY >= 0 && gridY < GRID_HEIGHT) {
            // Update item position
            if (selectedSlot >= 0 && selectedSlot < (int)items.size()) {
                items[selectedSlot].gridX = gridX;
                items[selectedSlot].gridY = gridY;
                TraceLog(LOG_INFO, "Moved item to grid position %d, %d", gridX, gridY);
            }
        }
    }

    // Right click to use/consume
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        for (size_t i = 0; i < items.size(); i++) {
            const auto& slot = items[i];
            const ItemDefinition* itemDef = ItemDatabase::getItem(slot.itemId);
            if (!itemDef) continue;

            Rectangle itemRect = {
                (float)(60 + slot.gridX * CELL_SIZE),
                (float)(180 + slot.gridY * CELL_SIZE),
                (float)(itemDef->gridWidth * CELL_SIZE),
                (float)(itemDef->gridHeight * CELL_SIZE)
            };

            if (CheckCollisionPointRec(mousePos, itemRect)) {
                TraceLog(LOG_INFO, "Right-clicked item: %s", itemDef->name.c_str());
                // TODO: Implement context menu or quick use
                return;
            }
        }
    }
}

int InventoryUI::getSlotAtPosition(int gridX, int gridY) {
    for (size_t i = 0; i < items.size(); i++) {
        const auto& slot = items[i];
        const ItemDefinition* itemDef = ItemDatabase::getItem(slot.itemId);
        if (!itemDef) continue;

        if (gridX >= slot.gridX && gridX < slot.gridX + itemDef->gridWidth &&
            gridY >= slot.gridY && gridY < slot.gridY + itemDef->gridHeight) {
            return (int)i;
        }
    }
    return -1;
}

void InventoryUI::onStateChanged(GameState newState) {}

} // namespace TDS
