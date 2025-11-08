#include "MerchantScene.h"
#include "../../engine/GameEngine.h"
#include "../../common/ItemDatabase.h"
#include <sstream>
#include <algorithm>
#include <iostream>

MerchantScene::MerchantScene()
    : playerRoubles(50000),  // Starting money
      currentMerchantId("prapor"),
      tradeMode(TradeMode::BUY),
      currentFilter(ItemFilter::ALL),
      scrollOffset(0),
      maxVisibleItems(10) {
}

void MerchantScene::onEnter() {
    std::cout << "[MerchantScene] Entering merchant trading" << std::endl;

    initializeMerchants();
    loadPlayerInventory();
    initializeUI();
    switchMerchant(currentMerchantId);
    updateMoneyDisplay();
}

void MerchantScene::onExit() {
    std::cout << "[MerchantScene] Exiting merchant" << std::endl;
}

void MerchantScene::onPause() {
    // Save transaction state if needed
}

void MerchantScene::onResume() {
    updateItemList();
    updateMoneyDisplay();
}

void MerchantScene::tick(float dt) {
    // Update UI animations if any
}

void MerchantScene::fixedTick(float fixedDt) {
    // No physics in merchant scene
}

void MerchantScene::handleInput(const InputState& input) {
    // Handle keyboard shortcuts
    if (input.keys['B']) {
        onBuyModeClicked();
    }
    if (input.keys['S']) {
        onSellModeClicked();
    }
    if (input.keys['F']) {
        onFilterClicked();
    }
    if (input.keys[27]) {  // ESC
        onBackClicked();
    }

    // Scroll with mouse wheel or arrow keys
    if (input.keys[38]) {  // Up arrow
        scrollOffset = std::max(0, scrollOffset - 1);
        updateItemList();
    }
    if (input.keys[40]) {  // Down arrow
        scrollOffset++;
        updateItemList();
    }
}

void MerchantScene::render() {
    // Render merchant UI
    // In a real implementation, this would use the render engine
}

void MerchantScene::initializeUI() {
    // Title
    titleText = std::make_shared<Text>("MerchantTitle", "TRADERS");
    titleText->setPosition(960, 50);
    titleText->setSize(48);
    titleText->setColor(Color(255, 255, 255));

    // Merchant info
    merchantNameText = std::make_shared<Text>("MerchantName", "");
    merchantNameText->setPosition(400, 150);
    merchantNameText->setSize(32);
    merchantNameText->setColor(Color(255, 220, 100));

    merchantLoyaltyText = std::make_shared<Text>("MerchantLoyalty", "");
    merchantLoyaltyText->setPosition(400, 200);
    merchantLoyaltyText->setSize(20);
    merchantLoyaltyText->setColor(Color(200, 200, 200));

    // Player money
    playerMoneyText = std::make_shared<Text>("PlayerMoney", "");
    playerMoneyText->setPosition(1400, 100);
    playerMoneyText->setSize(24);
    playerMoneyText->setColor(Color(100, 255, 100));

    // Cart display
    cartText = std::make_shared<Text>("CartText", "");
    cartText->setPosition(1400, 300);
    cartText->setSize(18);
    cartText->setColor(Color(220, 220, 220));

    // Back button
    backButton = std::make_shared<UIButton>("BackButton", "BACK");
    backButton->setPosition(100, 950);
    backButton->setSize(200, 60);
    backButton->setOnClick([this]() { onBackClicked(); });

    // Mode buttons
    buyModeButton = std::make_shared<UIButton>("BuyMode", "BUY");
    buyModeButton->setPosition(350, 950);
    buyModeButton->setSize(200, 60);
    buyModeButton->setOnClick([this]() { onBuyModeClicked(); });

    sellModeButton = std::make_shared<UIButton>("SellMode", "SELL");
    sellModeButton->setPosition(600, 950);
    sellModeButton->setSize(200, 60);
    sellModeButton->setOnClick([this]() { onSellModeClicked(); });

    // Filter button
    filterButton = std::make_shared<UIButton>("FilterButton", "FILTER: ALL");
    filterButton->setPosition(850, 950);
    filterButton->setSize(250, 60);
    filterButton->setOnClick([this]() { onFilterClicked(); });

    // Confirm trade button
    confirmTradeButton = std::make_shared<UIButton>("ConfirmTrade", "CONFIRM TRADE");
    confirmTradeButton->setPosition(1500, 950);
    confirmTradeButton->setSize(300, 60);
    confirmTradeButton->setOnClick([this]() { onConfirmTradeClicked(); });

    // Merchant selection buttons
    int merchantButtonY = 250;
    std::vector<std::string> merchantIds = {"prapor", "therapist", "skier", "peacekeeper"};
    for (const auto& merchantId : merchantIds) {
        auto button = std::make_shared<UIButton>("Merchant_" + merchantId, merchantId);
        button->setPosition(100, merchantButtonY);
        button->setSize(200, 50);
        button->setOnClick([this, merchantId]() { onMerchantClicked(merchantId); });
        merchantButtons.push_back(button);
        merchantButtonY += 60;
    }
}

void MerchantScene::initializeMerchants() {
    createPraporMerchant();
    createTherapistMerchant();
    createSkierMerchant();
    createPeacekeeperMerchant();
}

void MerchantScene::createPraporMerchant() {
    Merchant prapor;
    prapor.id = "prapor";
    prapor.name = "Prapor";
    prapor.description = "Weapons and ammunition specialist";
    prapor.loyaltyLevel = 1;
    prapor.priceBuyMultiplier = 0.4f;   // Buys at 40% of value
    prapor.priceSellMultiplier = 1.0f;  // Sells at 100% of value

    // Add weapons and ammo to inventory
    ItemDatabase& itemDb = ItemDatabase::getInstance();

    Item ak74 = itemDb.getItem("ak74");
    if (!ak74.id.empty()) prapor.inventory.push_back(ak74);

    Item m4a1 = itemDb.getItem("m4a1");
    if (!m4a1.id.empty()) prapor.inventory.push_back(m4a1);

    // Add some ammo
    for (int i = 0; i < 5; i++) {
        Item ammo = itemDb.getItem("ammo_545");
        if (!ammo.id.empty()) {
            ammo.stackSize = 60;
            prapor.inventory.push_back(ammo);
        }
    }

    merchants[prapor.id] = prapor;
}

void MerchantScene::createTherapistMerchant() {
    Merchant therapist;
    therapist.id = "therapist";
    therapist.name = "Therapist";
    therapist.description = "Medical supplies specialist";
    therapist.loyaltyLevel = 1;
    therapist.priceBuyMultiplier = 0.5f;   // Better buy price for meds
    therapist.priceSellMultiplier = 0.9f;  // Better sell price for meds

    // Add medical items
    ItemDatabase& itemDb = ItemDatabase::getInstance();

    for (int i = 0; i < 10; i++) {
        Item medkit = itemDb.getItem("medkit");
        if (!medkit.id.empty()) therapist.inventory.push_back(medkit);

        Item bandage = itemDb.getItem("bandage");
        if (!bandage.id.empty()) therapist.inventory.push_back(bandage);
    }

    merchants[therapist.id] = therapist;
}

void MerchantScene::createSkierMerchant() {
    Merchant skier;
    skier.id = "skier";
    skier.name = "Skier";
    skier.description = "Armor and equipment specialist";
    skier.loyaltyLevel = 1;
    skier.priceBuyMultiplier = 0.45f;
    skier.priceSellMultiplier = 1.1f;

    // Add armor items
    ItemDatabase& itemDb = ItemDatabase::getInstance();

    Item paca = itemDb.getItem("paca");
    if (!paca.id.empty()) skier.inventory.push_back(paca);

    Item fort = itemDb.getItem("fort_armor");
    if (!fort.id.empty()) skier.inventory.push_back(fort);

    merchants[skier.id] = skier;
}

void MerchantScene::createPeacekeeperMerchant() {
    Merchant peacekeeper;
    peacekeeper.id = "peacekeeper";
    peacekeeper.name = "Peacekeeper";
    peacekeeper.description = "Western equipment specialist";
    peacekeeper.loyaltyLevel = 1;
    peacekeeper.priceBuyMultiplier = 0.35f;  // Lower buy prices
    peacekeeper.priceSellMultiplier = 1.3f;  // Higher sell prices (imports)

    // Add western weapons
    ItemDatabase& itemDb = ItemDatabase::getInstance();

    Item m4a1 = itemDb.getItem("m4a1");
    if (!m4a1.id.empty()) peacekeeper.inventory.push_back(m4a1);

    merchants[peacekeeper.id] = peacekeeper;
}

void MerchantScene::loadPlayerInventory() {
    playerInventory = std::make_unique<Inventory>();
    // TODO: Load from save data
}

void MerchantScene::switchMerchant(const std::string& merchantId) {
    auto it = merchants.find(merchantId);
    if (it == merchants.end()) {
        std::cerr << "[MerchantScene] Merchant not found: " << merchantId << std::endl;
        return;
    }

    currentMerchantId = merchantId;
    const Merchant& merchant = it->second;

    // Update UI
    merchantNameText->setText(merchant.name);

    std::stringstream ss;
    ss << "Loyalty Level: " << merchant.loyaltyLevel << "\n";
    ss << merchant.description;
    merchantLoyaltyText->setText(ss.str());

    // Clear cart when switching merchants
    tradeCart.clear();
    updateCartDisplay();

    // Update item list
    updateItemList();

    std::cout << "[MerchantScene] Switched to merchant: " << merchant.name << std::endl;
}

void MerchantScene::updateItemList() {
    displayedItems.clear();

    auto it = merchants.find(currentMerchantId);
    if (it == merchants.end()) return;

    Merchant& merchant = it->second;

    // Get items based on trade mode
    std::vector<Item*> items;
    if (tradeMode == TradeMode::BUY) {
        // Show merchant's inventory
        for (auto& item : merchant.inventory) {
            items.push_back(&item);
        }
    }
    else {
        // Show player's inventory
        auto stashItems = playerInventory->getStash()->getAllItems();
        items.insert(items.end(), stashItems.begin(), stashItems.end());
    }

    // Apply filter
    std::vector<Item*> filteredItems;
    for (Item* item : items) {
        if (!item) continue;

        bool matches = true;
        switch (currentFilter) {
            case ItemFilter::WEAPONS:
                matches = (item->type == ItemType::WEAPON);
                break;
            case ItemFilter::ARMOR:
                matches = (item->type == ItemType::ARMOR);
                break;
            case ItemFilter::MEDICAL:
                matches = (item->type == ItemType::MEDICAL);
                break;
            case ItemFilter::FOOD:
                matches = (item->type == ItemType::FOOD);
                break;
            case ItemFilter::AMMO:
                matches = (item->type == ItemType::AMMO);
                break;
            case ItemFilter::LOOT:
                matches = (item->type == ItemType::LOOT);
                break;
            default:
                break;
        }

        if (matches) {
            filteredItems.push_back(item);
        }
    }

    // Create UI entries for visible items
    int itemY = 400;
    int visibleCount = 0;
    for (size_t i = scrollOffset; i < filteredItems.size() && visibleCount < maxVisibleItems; i++) {
        Item* item = filteredItems[i];

        ItemListEntry entry;
        entry.item = item;
        entry.price = calculateItemPrice(*item, tradeMode == TradeMode::BUY);
        entry.canAfford = (playerRoubles >= entry.price) || (tradeMode == TradeMode::SELL);

        // Create button for this item
        std::stringstream buttonLabel;
        buttonLabel << item->name << " - " << entry.price << " ₽";

        entry.button = std::make_shared<UIButton>("Item_" + std::to_string(i), buttonLabel.str());
        entry.button->setPosition(400, itemY);
        entry.button->setSize(800, 40);
        entry.button->setOnClick([this, item]() { onItemClicked(item); });

        displayedItems.push_back(entry);

        itemY += 45;
        visibleCount++;
    }
}

void MerchantScene::updateCartDisplay() {
    std::stringstream ss;
    ss << "SHOPPING CART\n\n";

    int totalPrice = 0;
    for (const auto& tradeItem : tradeCart) {
        ss << tradeItem.item.name << " x" << tradeItem.quantity;
        ss << " - " << tradeItem.totalPrice << " ₽\n";
        totalPrice += tradeItem.totalPrice;
    }

    ss << "\nTOTAL: " << totalPrice << " ₽\n";

    if (tradeMode == TradeMode::BUY) {
        ss << "You will pay: " << totalPrice << " ₽\n";
        ss << "Remaining: " << (playerRoubles - totalPrice) << " ₽";
    }
    else {
        ss << "You will receive: " << totalPrice << " ₽\n";
        ss << "New total: " << (playerRoubles + totalPrice) << " ₽";
    }

    cartText->setText(ss.str());
}

void MerchantScene::updateMoneyDisplay() {
    std::stringstream ss;
    ss << "₽ " << playerRoubles;
    playerMoneyText->setText(ss.str());
}

void MerchantScene::addItemToCart(Item* item) {
    if (!item) return;

    int price = calculateItemPrice(*item, tradeMode == TradeMode::BUY);

    // Check if item already in cart
    for (auto& tradeItem : tradeCart) {
        if (tradeItem.item.id == item->id) {
            tradeItem.quantity++;
            tradeItem.totalPrice += price;
            updateCartDisplay();
            return;
        }
    }

    // Add new item to cart
    TradeItem tradeItem;
    tradeItem.item = *item;
    tradeItem.quantity = 1;
    tradeItem.totalPrice = price;
    tradeCart.push_back(tradeItem);

    updateCartDisplay();
    std::cout << "[MerchantScene] Added " << item->name << " to cart" << std::endl;
}

void MerchantScene::removeItemFromCart(int cartIndex) {
    if (cartIndex >= 0 && cartIndex < static_cast<int>(tradeCart.size())) {
        tradeCart.erase(tradeCart.begin() + cartIndex);
        updateCartDisplay();
    }
}

void MerchantScene::confirmTrade() {
    if (tradeCart.empty()) {
        std::cout << "[MerchantScene] Cart is empty" << std::endl;
        return;
    }

    if (!canAffordTrade()) {
        std::cout << "[MerchantScene] Cannot afford trade" << std::endl;
        return;
    }

    int totalPrice = 0;
    for (const auto& tradeItem : tradeCart) {
        totalPrice += tradeItem.totalPrice;
    }

    if (tradeMode == TradeMode::BUY) {
        // Buy from merchant
        playerRoubles -= totalPrice;

        // Add items to player inventory
        for (const auto& tradeItem : tradeCart) {
            for (int i = 0; i < tradeItem.quantity; i++) {
                playerInventory->getStash()->addItem(tradeItem.item);
            }
        }

        std::cout << "[MerchantScene] Purchased " << tradeCart.size() << " items for " << totalPrice << " ₽" << std::endl;
    }
    else {
        // Sell to merchant
        playerRoubles += totalPrice;

        // Remove items from player inventory
        for (const auto& tradeItem : tradeCart) {
            for (int i = 0; i < tradeItem.quantity; i++) {
                // Find and remove item
                auto stashItems = playerInventory->getStash()->getAllItems();
                for (Item* item : stashItems) {
                    if (item->id == tradeItem.item.id) {
                        playerInventory->getStash()->removeItem(item->instanceId);
                        break;
                    }
                }
            }
        }

        std::cout << "[MerchantScene] Sold " << tradeCart.size() << " items for " << totalPrice << " ₽" << std::endl;
    }

    // Clear cart
    tradeCart.clear();
    updateCartDisplay();
    updateMoneyDisplay();
    updateItemList();
}

bool MerchantScene::canAffordTrade() const {
    if (tradeMode == TradeMode::SELL) {
        return true;  // Can always sell
    }

    int totalPrice = 0;
    for (const auto& tradeItem : tradeCart) {
        totalPrice += tradeItem.totalPrice;
    }

    return playerRoubles >= totalPrice;
}

int MerchantScene::calculateItemPrice(const Item& item, bool buying) const {
    auto it = merchants.find(currentMerchantId);
    if (it == merchants.end()) return item.value;

    const Merchant& merchant = it->second;

    if (buying) {
        // Player is buying from merchant
        return static_cast<int>(item.value * merchant.priceSellMultiplier);
    }
    else {
        // Player is selling to merchant
        return static_cast<int>(item.value * merchant.priceBuyMultiplier);
    }
}

void MerchantScene::onBackClicked() {
    std::cout << "[MerchantScene] Back button clicked" << std::endl;

    // Return to main menu
    SceneManager* sceneManager = ENGINE.getSceneManager();
    if (sceneManager) {
        sceneManager->switchScene("menu");
    }
}

void MerchantScene::onBuyModeClicked() {
    tradeMode = TradeMode::BUY;
    tradeCart.clear();
    updateItemList();
    updateCartDisplay();
    std::cout << "[MerchantScene] Switched to BUY mode" << std::endl;
}

void MerchantScene::onSellModeClicked() {
    tradeMode = TradeMode::SELL;
    tradeCart.clear();
    updateItemList();
    updateCartDisplay();
    std::cout << "[MerchantScene] Switched to SELL mode" << std::endl;
}

void MerchantScene::onConfirmTradeClicked() {
    confirmTrade();
}

void MerchantScene::onFilterClicked() {
    // Cycle through filters
    switch (currentFilter) {
        case ItemFilter::ALL:
            currentFilter = ItemFilter::WEAPONS;
            filterButton->setLabel("FILTER: WEAPONS");
            break;
        case ItemFilter::WEAPONS:
            currentFilter = ItemFilter::ARMOR;
            filterButton->setLabel("FILTER: ARMOR");
            break;
        case ItemFilter::ARMOR:
            currentFilter = ItemFilter::MEDICAL;
            filterButton->setLabel("FILTER: MEDICAL");
            break;
        case ItemFilter::MEDICAL:
            currentFilter = ItemFilter::FOOD;
            filterButton->setLabel("FILTER: FOOD");
            break;
        case ItemFilter::FOOD:
            currentFilter = ItemFilter::AMMO;
            filterButton->setLabel("FILTER: AMMO");
            break;
        case ItemFilter::AMMO:
            currentFilter = ItemFilter::LOOT;
            filterButton->setLabel("FILTER: LOOT");
            break;
        case ItemFilter::LOOT:
            currentFilter = ItemFilter::ALL;
            filterButton->setLabel("FILTER: ALL");
            break;
    }

    scrollOffset = 0;
    updateItemList();
}

void MerchantScene::onMerchantClicked(const std::string& merchantId) {
    switchMerchant(merchantId);
}

void MerchantScene::onItemClicked(Item* item) {
    addItemToCart(item);
}
