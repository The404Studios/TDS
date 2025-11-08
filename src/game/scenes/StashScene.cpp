#include "StashScene.h"
#include "../../engine/GameEngine.h"
#include <sstream>
#include <algorithm>
#include <iostream>

StashScene::StashScene()
    : selectedItem(nullptr),
      currentSort(SortMode::NONE),
      currentFilter(FilterMode::ALL),
      cellSize(50.0f),
      gridStartX(100.0f),
      gridStartY(150.0f),
      gridPadding(2.0f) {
}

void StashScene::onEnter() {
    std::cout << "[StashScene] Entering stash management" << std::endl;

    initializeInventory();
    initializeUI();
    updateGridCells();
    updateStatsText();
}

void StashScene::onExit() {
    std::cout << "[StashScene] Exiting stash" << std::endl;
}

void StashScene::onPause() {
    // Save inventory state
}

void StashScene::onResume() {
    updateGridCells();
    updateStatsText();
}

void StashScene::tick(float dt) {
    // Update UI animations if any
}

void StashScene::fixedTick(float fixedDt) {
    // No physics in stash scene
}

void StashScene::handleInput(const InputState& input) {
    // Handle mouse input for drag and drop
    if (input.mouseLeftPressed && !dragState.dragging) {
        handleMouseClick(input.mouseX, input.mouseY);
    }
    else if (dragState.dragging) {
        if (input.mouseLeftPressed) {
            handleMouseDrag(input.mouseX, input.mouseY);
        }
        else {
            handleMouseRelease(input.mouseX, input.mouseY);
        }
    }

    // Handle keyboard shortcuts
    if (input.keys['F']) {
        onFilterClicked();
    }
    if (input.keys['S']) {
        onSortClicked();
    }
    if (input.keys[27]) {  // ESC
        onBackClicked();
    }
}

void StashScene::render() {
    renderGrid();
    renderItems();
    if (dragState.dragging) {
        renderDraggedItem();
    }
    if (selectedItem) {
        renderItemDetails();
    }

    // Render UI elements (buttons, text)
    // In a real implementation, this would use the render engine
    // For now, this is a stub
}

void StashScene::initializeUI() {
    // Title
    titleText = std::make_shared<Text>("StashTitle", "STASH");
    titleText->setPosition(960, 50);
    titleText->setSize(48);
    titleText->setColor(Color(255, 255, 255));

    // Stats text
    statsText = std::make_shared<Text>("StatsText", "");
    statsText->setPosition(1400, 100);
    statsText->setSize(20);
    statsText->setColor(Color(200, 200, 200));

    // Item details text
    itemDetailsText = std::make_shared<Text>("ItemDetails", "");
    itemDetailsText->setPosition(1400, 300);
    itemDetailsText->setSize(18);
    itemDetailsText->setColor(Color(220, 220, 220));

    // Back button
    backButton = std::make_shared<UIButton>("BackButton", "BACK");
    backButton->setPosition(100, 900);
    backButton->setSize(200, 60);
    backButton->setOnClick([this]() { onBackClicked(); });

    // Sort button
    sortButton = std::make_shared<UIButton>("SortButton", "SORT");
    sortButton->setPosition(350, 900);
    sortButton->setSize(200, 60);
    sortButton->setOnClick([this]() { onSortClicked(); });

    // Filter button
    filterButton = std::make_shared<UIButton>("FilterButton", "FILTER: ALL");
    filterButton->setPosition(600, 900);
    filterButton->setSize(250, 60);
    filterButton->setOnClick([this]() { onFilterClicked(); });
}

void StashScene::initializeInventory() {
    inventory = std::make_unique<Inventory>();

    // TODO: Load inventory from player save data
    // For now, add some test items
    ItemDatabase& itemDb = ItemDatabase::getInstance();

    // Add some sample items for testing
    Item ak74 = itemDb.getItem("ak74");
    if (!ak74.id.empty()) {
        inventory->getStash()->addItem(ak74);
    }

    Item medkit = itemDb.getItem("medkit");
    if (!medkit.id.empty()) {
        inventory->getStash()->addItem(medkit);
    }
}

void StashScene::updateGridCells() {
    gridCells.clear();

    InventoryGrid* stash = inventory->getStash();
    if (!stash) return;

    // Create cells for the entire grid
    for (int y = 0; y < stash->getHeight(); y++) {
        for (int x = 0; x < stash->getWidth(); x++) {
            GridCell cell;
            cell.x = x;
            cell.y = y;

            Item* item = stash->getItemAt(x, y);
            cell.occupied = (item != nullptr);
            cell.itemInstanceId = item ? item->instanceId : 0;

            // Color based on state
            if (cell.occupied) {
                // Color by rarity
                if (item) {
                    switch (item->rarity) {
                        case ItemRarity::LEGENDARY:
                            cell.color = Color(255, 165, 0);  // Orange
                            break;
                        case ItemRarity::RARE:
                            cell.color = Color(100, 100, 255);  // Blue
                            break;
                        case ItemRarity::UNCOMMON:
                            cell.color = Color(100, 255, 100);  // Green
                            break;
                        default:
                            cell.color = Color(150, 150, 150);  // Gray
                            break;
                    }
                }
            } else {
                cell.color = Color(40, 40, 40);  // Dark gray for empty
            }

            gridCells.push_back(cell);
        }
    }
}

void StashScene::renderGrid() {
    // Render grid background
    // In a real implementation, this would use RenderEngine
    // For now, this is a stub that would draw:
    // - Grid lines
    // - Cell backgrounds based on gridCells

    std::cout << "[StashScene] Rendering " << gridCells.size() << " grid cells" << std::endl;
}

void StashScene::renderItems() {
    InventoryGrid* stash = inventory->getStash();
    if (!stash) return;

    auto items = stash->getAllItems();

    // Render each item sprite/icon
    for (Item* item : items) {
        if (!item) continue;

        // Find grid position (would need to track this properly)
        // For now, just render at the first cell that contains this item
        for (const auto& cell : gridCells) {
            if (cell.itemInstanceId == item->instanceId) {
                int screenX, screenY;
                gridToScreen(cell.x, cell.y, screenX, screenY);

                // Render item sprite/icon at screenX, screenY
                // Would use RenderEngine in real implementation
                break;
            }
        }
    }
}

void StashScene::renderDraggedItem() {
    if (!dragState.dragging) return;

    Item* item = inventory->getStash()->getItemByInstanceId(dragState.itemInstanceId);
    if (!item) return;

    // Render item at mouse position with transparency
    // Would use RenderEngine in real implementation
}

void StashScene::renderItemDetails() {
    if (!selectedItem) return;

    std::stringstream ss;
    ss << selectedItem->name << "\n";
    ss << "Type: " << static_cast<int>(selectedItem->type) << "\n";
    ss << "Value: " << selectedItem->value << " ₽\n";
    ss << "Weight: " << selectedItem->weight << " kg\n";
    ss << "Size: " << selectedItem->width << "x" << selectedItem->height << "\n";

    if (selectedItem->type == ItemType::WEAPON) {
        ss << "\nDamage: " << selectedItem->damage << "\n";
        ss << "Fire Rate: " << selectedItem->fireRate << "\n";
        ss << "Magazine: " << selectedItem->magazineSize << "\n";
    }
    else if (selectedItem->type == ItemType::ARMOR) {
        ss << "\nArmor Class: " << selectedItem->armorClass << "\n";
        ss << "Durability: " << selectedItem->durability << "/" << selectedItem->maxDurability << "\n";
    }
    else if (selectedItem->type == ItemType::MEDICAL) {
        ss << "\nHeal Amount: " << selectedItem->healAmount << "\n";
        ss << "Use Time: " << selectedItem->useTime << "s\n";
    }

    itemDetailsText->setText(ss.str());
}

void StashScene::updateStatsText() {
    InventoryGrid* stash = inventory->getStash();
    if (!stash) return;

    float weight = inventory->getTotalWeight();
    int value = inventory->getTotalValue();
    int usedSlots = stash->getUsedSlots();
    int totalSlots = stash->getTotalSlots();
    float usagePercent = stash->getUsagePercent();

    std::stringstream ss;
    ss << "Total Weight: " << weight << " kg\n";
    ss << "Total Value: " << value << " ₽\n";
    ss << "Slots Used: " << usedSlots << "/" << totalSlots << " (" << usagePercent << "%)\n";

    statsText->setText(ss.str());
}

void StashScene::handleMouseClick(int mouseX, int mouseY) {
    int gridX, gridY;
    screenToGrid(mouseX, mouseY, gridX, gridY);

    InventoryGrid* stash = inventory->getStash();
    if (!stash) return;

    Item* item = stash->getItemAt(gridX, gridY);
    if (item) {
        // Start dragging
        dragState.dragging = true;
        dragState.itemInstanceId = item->instanceId;
        dragState.startGridX = gridX;
        dragState.startGridY = gridY;
        dragState.currentMouseX = mouseX;
        dragState.currentMouseY = mouseY;

        // Select item for details view
        selectedItem = item;
    }
    else {
        selectedItem = nullptr;
    }
}

void StashScene::handleMouseDrag(int mouseX, int mouseY) {
    dragState.currentMouseX = mouseX;
    dragState.currentMouseY = mouseY;
}

void StashScene::handleMouseRelease(int mouseX, int mouseY) {
    if (!dragState.dragging) return;

    int gridX, gridY;
    screenToGrid(mouseX, mouseY, gridX, gridY);

    InventoryGrid* stash = inventory->getStash();
    if (stash) {
        // Try to move item to new position
        if (stash->moveItem(dragState.itemInstanceId, gridX, gridY)) {
            std::cout << "[StashScene] Moved item to (" << gridX << ", " << gridY << ")" << std::endl;
            updateGridCells();
        }
        else {
            std::cout << "[StashScene] Failed to move item to (" << gridX << ", " << gridY << ")" << std::endl;
        }
    }

    // Clear drag state
    dragState.dragging = false;
    dragState.itemInstanceId = 0;
}

void StashScene::screenToGrid(int screenX, int screenY, int& gridX, int& gridY) {
    gridX = static_cast<int>((screenX - gridStartX) / (cellSize + gridPadding));
    gridY = static_cast<int>((screenY - gridStartY) / (cellSize + gridPadding));

    // Clamp to grid bounds
    InventoryGrid* stash = inventory->getStash();
    if (stash) {
        if (gridX < 0) gridX = 0;
        if (gridY < 0) gridY = 0;
        if (gridX >= stash->getWidth()) gridX = stash->getWidth() - 1;
        if (gridY >= stash->getHeight()) gridY = stash->getHeight() - 1;
    }
}

void StashScene::gridToScreen(int gridX, int gridY, int& screenX, int& screenY) {
    screenX = static_cast<int>(gridStartX + gridX * (cellSize + gridPadding));
    screenY = static_cast<int>(gridStartY + gridY * (cellSize + gridPadding));
}

void StashScene::onBackClicked() {
    std::cout << "[StashScene] Back button clicked" << std::endl;

    // Return to main menu
    SceneManager* sceneManager = ENGINE.getSceneManager();
    if (sceneManager) {
        sceneManager->switchScene("menu");
    }
}

void StashScene::onSortClicked() {
    // Cycle through sort modes
    switch (currentSort) {
        case SortMode::NONE:
            currentSort = SortMode::BY_VALUE;
            sortButton->setLabel("SORT: VALUE");
            break;
        case SortMode::BY_VALUE:
            currentSort = SortMode::BY_NAME;
            sortButton->setLabel("SORT: NAME");
            break;
        case SortMode::BY_NAME:
            currentSort = SortMode::BY_TYPE;
            sortButton->setLabel("SORT: TYPE");
            break;
        case SortMode::BY_TYPE:
            currentSort = SortMode::BY_WEIGHT;
            sortButton->setLabel("SORT: WEIGHT");
            break;
        case SortMode::BY_WEIGHT:
            currentSort = SortMode::NONE;
            sortButton->setLabel("SORT: NONE");
            break;
    }

    applySorting();
}

void StashScene::onFilterClicked() {
    // Cycle through filter modes
    switch (currentFilter) {
        case FilterMode::ALL:
            currentFilter = FilterMode::WEAPONS;
            filterButton->setLabel("FILTER: WEAPONS");
            break;
        case FilterMode::WEAPONS:
            currentFilter = FilterMode::ARMOR;
            filterButton->setLabel("FILTER: ARMOR");
            break;
        case FilterMode::ARMOR:
            currentFilter = FilterMode::MEDICAL;
            filterButton->setLabel("FILTER: MEDICAL");
            break;
        case FilterMode::MEDICAL:
            currentFilter = FilterMode::FOOD;
            filterButton->setLabel("FILTER: FOOD");
            break;
        case FilterMode::FOOD:
            currentFilter = FilterMode::AMMO;
            filterButton->setLabel("FILTER: AMMO");
            break;
        case FilterMode::AMMO:
            currentFilter = FilterMode::LOOT;
            filterButton->setLabel("FILTER: LOOT");
            break;
        case FilterMode::LOOT:
            currentFilter = FilterMode::ALL;
            filterButton->setLabel("FILTER: ALL");
            break;
    }

    applyFiltering();
}

void StashScene::applySorting() {
    // TODO: Implement actual sorting
    // This would reorganize items in the grid based on the selected sort mode
    std::cout << "[StashScene] Applying sort mode: " << static_cast<int>(currentSort) << std::endl;
    updateGridCells();
}

void StashScene::applyFiltering() {
    // TODO: Implement actual filtering
    // This would hide/show items based on the selected filter
    std::cout << "[StashScene] Applying filter mode: " << static_cast<int>(currentFilter) << std::endl;
    updateGridCells();
}

std::vector<Item*> StashScene::getFilteredItems() {
    InventoryGrid* stash = inventory->getStash();
    if (!stash) return {};

    auto allItems = stash->getAllItems();
    if (currentFilter == FilterMode::ALL) {
        return allItems;
    }

    std::vector<Item*> filtered;
    for (Item* item : allItems) {
        if (!item) continue;

        bool matches = false;
        switch (currentFilter) {
            case FilterMode::WEAPONS:
                matches = (item->type == ItemType::WEAPON);
                break;
            case FilterMode::ARMOR:
                matches = (item->type == ItemType::ARMOR);
                break;
            case FilterMode::MEDICAL:
                matches = (item->type == ItemType::MEDICAL);
                break;
            case FilterMode::FOOD:
                matches = (item->type == ItemType::FOOD);
                break;
            case FilterMode::AMMO:
                matches = (item->type == ItemType::AMMO);
                break;
            case FilterMode::LOOT:
                matches = (item->type == ItemType::LOOT);
                break;
            default:
                break;
        }

        if (matches) {
            filtered.push_back(item);
        }
    }

    return filtered;
}
