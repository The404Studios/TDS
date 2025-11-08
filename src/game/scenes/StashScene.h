#pragma once

#include "../../engine/scene/Scene.h"
#include "../../client/ui/GameObject.h"
#include "../../client/ui/UIButton.h"
#include "../../client/ui/Text.h"
#include "../systems/InventorySystem.h"
#include <memory>
#include <vector>

/**
 * StashScene - Persistent inventory management screen
 * Like Tarkov's stash where you organize loot between raids
 */
class StashScene : public Scene {
public:
    StashScene();
    ~StashScene() override = default;

    // Scene lifecycle
    void onEnter() override;
    void onExit() override;
    void onPause() override;
    void onResume() override;

    // Update
    void tick(float dt) override;
    void fixedTick(float fixedDt) override;

    // Input
    void handleInput(const InputState& input) override;

    // Rendering
    void render() override;

private:
    // Inventory system
    std::unique_ptr<Inventory> inventory;

    // UI Elements
    std::shared_ptr<Text> titleText;
    std::shared_ptr<Text> statsText;  // Weight, value, slots used
    std::shared_ptr<UIButton> backButton;
    std::shared_ptr<UIButton> sortButton;
    std::shared_ptr<UIButton> filterButton;

    // Grid rendering
    struct GridCell {
        int x, y;
        bool occupied;
        uint32_t itemInstanceId;
        Color color;
    };
    std::vector<GridCell> gridCells;

    // Grid interaction
    struct DragState {
        bool dragging;
        uint32_t itemInstanceId;
        int startGridX, startGridY;
        int currentMouseX, currentMouseY;
        DragState() : dragging(false), itemInstanceId(0), startGridX(0), startGridY(0),
                     currentMouseX(0), currentMouseY(0) {}
    };
    DragState dragState;

    // Selected item
    Item* selectedItem;
    std::shared_ptr<Text> itemDetailsText;

    // Filter/Sort
    enum class SortMode {
        NONE,
        BY_VALUE,
        BY_NAME,
        BY_TYPE,
        BY_WEIGHT
    };
    SortMode currentSort;

    enum class FilterMode {
        ALL,
        WEAPONS,
        ARMOR,
        MEDICAL,
        FOOD,
        AMMO,
        LOOT
    };
    FilterMode currentFilter;

    // Grid display settings
    float cellSize;
    float gridStartX, gridStartY;
    float gridPadding;

    // Methods
    void initializeUI();
    void initializeInventory();
    void updateGridCells();
    void renderGrid();
    void renderItems();
    void renderDraggedItem();
    void renderItemDetails();
    void updateStatsText();

    // Input handling
    void handleMouseClick(int mouseX, int mouseY);
    void handleMouseDrag(int mouseX, int mouseY);
    void handleMouseRelease(int mouseX, int mouseY);
    void screenToGrid(int screenX, int screenY, int& gridX, int& gridY);
    void gridToScreen(int gridX, int gridY, int& screenX, int& screenY);

    // UI callbacks
    void onBackClicked();
    void onSortClicked();
    void onFilterClicked();

    // Sorting and filtering
    void applySorting();
    void applyFiltering();
    std::vector<Item*> getFilteredItems();
};
