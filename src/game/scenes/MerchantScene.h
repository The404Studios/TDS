#pragma once

#include "../../engine/scene/IScene.h"
#include "../../client/ui/GameObject.h"
#include "../../client/ui/UIButton.h"
#include "../../client/ui/Text.h"
#include "../systems/InventorySystem.h"
#include <memory>
#include <vector>
#include <map>

/**
 * Merchant data - represents a trader NPC
 */
struct Merchant {
    std::string id;
    std::string name;
    std::string description;
    int loyaltyLevel;  // 1-4
    float priceBuyMultiplier;   // How much they pay for items (0.3 = 30% of value)
    float priceSellMultiplier;  // How much they charge (1.2 = 120% of value)
    std::vector<Item> inventory;  // Items they sell

    Merchant()
        : loyaltyLevel(1), priceBuyMultiplier(0.4f), priceSellMultiplier(1.0f) {}
};

/**
 * MerchantScene - Trading interface with NPCs
 * Like Tarkov's merchant screens (Prapor, Therapist, etc.)
 */
class MerchantScene : public IScene {
public:
    MerchantScene();
    ~MerchantScene() override = default;

    // IScene interface
    std::string name() const override { return "Merchant"; }
    bool onEnter() override;
    void onExit() override;
    void fixedUpdate(float dt) override;
    void update(float dt) override;
    void render() override;

    // Custom input handling
    void handleInput(const InputState& input);

private:
    // Player inventory
    std::unique_ptr<Inventory> playerInventory;
    int playerRoubles;

    // Merchants
    std::map<std::string, Merchant> merchants;
    std::string currentMerchantId;

    // UI State
    enum class TradeMode {
        BUY,   // Buying from merchant
        SELL   // Selling to merchant
    };
    TradeMode tradeMode;

    // UI Elements
    std::shared_ptr<Text> titleText;
    std::shared_ptr<Text> merchantNameText;
    std::shared_ptr<Text> merchantLoyaltyText;
    std::shared_ptr<Text> playerMoneyText;
    std::shared_ptr<UIButton> backButton;
    std::shared_ptr<UIButton> buyModeButton;
    std::shared_ptr<UIButton> sellModeButton;
    std::shared_ptr<UIButton> confirmTradeButton;

    // Merchant selection buttons
    std::vector<std::shared_ptr<UIButton>> merchantButtons;

    // Item list UI
    struct ItemListEntry {
        Item* item;
        int price;
        bool canAfford;
        std::shared_ptr<UIButton> button;
    };
    std::vector<ItemListEntry> displayedItems;

    // Selected items for trade
    struct TradeItem {
        Item item;
        int quantity;
        int totalPrice;
    };
    std::vector<TradeItem> tradeCart;
    std::shared_ptr<Text> cartText;

    // Filtering
    enum class ItemFilter {
        ALL,
        WEAPONS,
        ARMOR,
        MEDICAL,
        FOOD,
        AMMO,
        LOOT
    };
    ItemFilter currentFilter;
    std::shared_ptr<UIButton> filterButton;

    // Scrolling
    int scrollOffset;
    int maxVisibleItems;

    // Methods
    void initializeUI();
    void initializeMerchants();
    void loadPlayerInventory();
    void switchMerchant(const std::string& merchantId);
    void updateItemList();
    void updateCartDisplay();
    void updateMoneyDisplay();

    // Trading
    void addItemToCart(Item* item);
    void removeItemFromCart(int cartIndex);
    void confirmTrade();
    bool canAffordTrade() const;
    int calculateItemPrice(const Item& item, bool buying) const;

    // UI Callbacks
    void onBackClicked();
    void onBuyModeClicked();
    void onSellModeClicked();
    void onConfirmTradeClicked();
    void onFilterClicked();
    void onMerchantClicked(const std::string& merchantId);
    void onItemClicked(Item* item);

    // Merchant initialization
    void createPraporMerchant();
    void createTherapistMerchant();
    void createSkierMerchant();
    void createPeacekeeperMerchant();
};
