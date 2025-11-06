#include "MerchantManager.h"
#include <iostream>

MerchantManager::MerchantManager(PersistenceManager* persistMgr) : persistenceManager(persistMgr) {
    initializeMerchants();
}

std::vector<MerchantOffer> MerchantManager::getMerchantOffers(MerchantType merchantType) {
    auto it = merchants.find(merchantType);
    if (it != merchants.end()) {
        return it->second.offers;
    }
    return std::vector<MerchantOffer>();
}

bool MerchantManager::buyItem(uint64_t accountId, MerchantType merchantType, const std::string& itemId,
                              int quantity, std::string& errorMsg) {
    // Get player data
    PlayerData* playerData = persistenceManager->getPlayerData(accountId);
    if (!playerData) {
        errorMsg = "Player data not found";
        return false;
    }

    // Get merchant
    auto it = merchants.find(merchantType);
    if (it == merchants.end()) {
        errorMsg = "Merchant not found";
        return false;
    }

    Merchant& merchant = it->second;

    // Find item in merchant offers
    MerchantOffer* offer = nullptr;
    for (auto& o : merchant.offers) {
        if (itemId == o.itemName) {  // Using itemName as ID for now
            offer = &o;
            break;
        }
    }

    if (!offer) {
        errorMsg = "Item not available from this merchant";
        return false;
    }

    // Check stock
    if (offer->stock > 0 && quantity > offer->stock) {
        errorMsg = "Insufficient stock";
        return false;
    }

    // Calculate total cost
    int totalCost = static_cast<int>(offer->price * quantity * merchant.sellPriceMultiplier);

    // Check if player has enough money
    if (playerData->stats.roubles < totalCost) {
        errorMsg = "Insufficient funds";
        return false;
    }

    // Deduct money
    playerData->stats.roubles -= totalCost;

    // Add items to stash
    auto& itemDb = ItemDatabase::getInstance();
    Item* itemTemplate = itemDb.getItemTemplate(itemId);

    if (!itemTemplate) {
        errorMsg = "Item template not found";
        return false;
    }

    for (int i = 0; i < quantity; i++) {
        Item item = itemDb.createItem(itemId);
        item.foundInRaid = false;  // Bought items are not FiR
        playerData->stash.push_back(item);
    }

    // Reduce stock (if limited)
    if (offer->stock > 0) {
        offer->stock -= quantity;
    }

    // Save player data
    persistenceManager->savePlayerData(accountId);

    std::cout << "[MerchantManager] Player " << accountId << " bought " << quantity << "x "
              << itemId << " from " << merchant.name << " for " << totalCost << " roubles" << std::endl;

    return true;
}

bool MerchantManager::sellItem(uint64_t accountId, MerchantType merchantType, uint32_t itemInstanceId,
                               std::string& errorMsg) {
    // Get player data
    PlayerData* playerData = persistenceManager->getPlayerData(accountId);
    if (!playerData) {
        errorMsg = "Player data not found";
        return false;
    }

    // Get merchant
    auto it = merchants.find(merchantType);
    if (it == merchants.end()) {
        errorMsg = "Merchant not found";
        return false;
    }

    Merchant& merchant = it->second;

    // Find item in player's stash
    auto itemIt = std::find_if(playerData->stash.begin(), playerData->stash.end(),
        [itemInstanceId](const Item& item) { return item.instanceId == itemInstanceId; });

    if (itemIt == playerData->stash.end()) {
        errorMsg = "Item not found in stash";
        return false;
    }

    Item& item = *itemIt;

    // Calculate sell price
    float multiplier = merchant.buyPriceMultiplier;
    if (item.foundInRaid) {
        multiplier *= 1.5f;  // FiR items sell for 50% more
    }

    int sellPrice = static_cast<int>(item.value * multiplier);

    // Add money to player
    playerData->stats.roubles += sellPrice;

    // Remove item from stash
    playerData->stash.erase(itemIt);

    // Save player data
    persistenceManager->savePlayerData(accountId);

    std::cout << "[MerchantManager] Player " << accountId << " sold "
              << item.name << " to " << merchant.name << " for " << sellPrice << " roubles" << std::endl;

    return true;
}

Merchant* MerchantManager::getMerchant(MerchantType type) {
    auto it = merchants.find(type);
    if (it != merchants.end()) {
        return &it->second;
    }
    return nullptr;
}

void MerchantManager::initializeMerchants() {
    auto& itemDb = ItemDatabase::getInstance();

    // FENCE - Buys/sells everything at low prices
    {
        Merchant fence;
        fence.type = MerchantType::FENCE;
        fence.name = "Fence";
        fence.buyPriceMultiplier = 0.4f;   // Buys at 40% value
        fence.sellPriceMultiplier = 1.8f;   // Sells at 180% value

        // Fence sells random junk items
        addOffer(fence, "bolts", 0, 1.0f);
        addOffer(fence, "wires", 0, 1.0f);
        addOffer(fence, "screw_nuts", 0, 1.0f);
        addOffer(fence, "tushonka", 0, 1.2f);
        addOffer(fence, "water", 0, 1.0f);
        addOffer(fence, "ai2", 0, 1.3f);

        merchants[MerchantType::FENCE] = fence;
    }

    // PRAPOR - Weapons & Ammo specialist
    {
        Merchant prapor;
        prapor.type = MerchantType::PRAPOR;
        prapor.name = "Prapor";
        prapor.buyPriceMultiplier = 0.6f;   // Buys at 60% value
        prapor.sellPriceMultiplier = 1.0f;  // Sells at base value

        // Prapor sells weapons and ammo
        addOffer(prapor, "ak74", 0, 1.0f);
        addOffer(prapor, "kedr", 0, 1.0f);
        addOffer(prapor, "sks", 0, 1.0f);
        addOffer(prapor, "545x39", 0, 1.0f);
        addOffer(prapor, "762x39", 0, 1.0f);
        addOffer(prapor, "9x18", 0, 1.0f);

        merchants[MerchantType::PRAPOR] = prapor;
    }

    // THERAPIST - Medical supplies
    {
        Merchant therapist;
        therapist.type = MerchantType::THERAPIST;
        therapist.name = "Therapist";
        therapist.buyPriceMultiplier = 0.7f;   // Buys at 70% value
        therapist.sellPriceMultiplier = 1.2f;  // Sells at 120% value

        // Therapist sells medical items
        addOffer(therapist, "ai2", 0, 1.0f);
        addOffer(therapist, "ifak", 0, 1.0f);
        addOffer(therapist, "salewa", 0, 1.0f);
        addOffer(therapist, "grizzly", 5, 1.0f);  // Limited stock
        addOffer(therapist, "morphine", 10, 1.0f);

        merchants[MerchantType::THERAPIST] = therapist;
    }

    // PEACEKEEPER - Western gear (expensive)
    {
        Merchant peacekeeper;
        peacekeeper.type = MerchantType::PEACEKEEPER;
        peacekeeper.name = "Peacekeeper";
        peacekeeper.buyPriceMultiplier = 0.5f;   // Buys at 50% value
        peacekeeper.sellPriceMultiplier = 1.5f;  // Sells at 150% value (expensive!)

        // Peacekeeper sells western weapons
        addOffer(peacekeeper, "m4a1", 0, 1.0f);
        addOffer(peacekeeper, "mp5", 0, 1.0f);
        addOffer(peacekeeper, "glock17", 0, 1.0f);
        addOffer(peacekeeper, "556x45", 0, 1.0f);
        addOffer(peacekeeper, "9x19", 0, 1.0f);

        merchants[MerchantType::PEACEKEEPER] = peacekeeper;
    }

    // RAGMAN - Armor & Clothing
    {
        Merchant ragman;
        ragman.type = MerchantType::RAGMAN;
        ragman.name = "Ragman";
        ragman.buyPriceMultiplier = 0.6f;   // Buys at 60% value
        ragman.sellPriceMultiplier = 1.1f;  // Sells at 110% value

        // Ragman sells armor and bags
        addOffer(ragman, "paca", 0, 1.0f);
        addOffer(ragman, "6b3", 0, 1.0f);
        addOffer(ragman, "ssh68", 0, 1.0f);
        addOffer(ragman, "zsh", 0, 1.0f);
        addOffer(ragman, "scav", 0, 1.0f);
        addOffer(ragman, "berkut", 0, 1.0f);
        addOffer(ragman, "trizip", 3, 1.0f);  // Limited stock

        merchants[MerchantType::RAGMAN] = ragman;
    }

    std::cout << "[MerchantManager] Initialized " << merchants.size() << " merchants" << std::endl;
}

void MerchantManager::addOffer(Merchant& merchant, const std::string& itemId, int stock, float markup) {
    auto& itemDb = ItemDatabase::getInstance();
    Item* itemTemplate = itemDb.getItemTemplate(itemId);

    if (itemTemplate) {
        MerchantOffer offer;
        offer.itemId = static_cast<uint32_t>(std::hash<std::string>{}(itemId));
        offer.itemName = itemId;  // Using ID as name for simplicity
        offer.price = itemTemplate->value;
        offer.stock = stock;
        offer.markup = markup;
        merchant.offers.push_back(offer);
    }
}
