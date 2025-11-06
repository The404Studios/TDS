#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include "../Common/ItemDatabase.h"
#include "PersistenceManager.h"
#include <map>
#include <vector>
#include <string>

// Merchant Manager - handles merchant trading, buy/sell operations
class MerchantManager {
public:
    MerchantManager(PersistenceManager* persistMgr);

    // Get merchant offers
    std::vector<MerchantOffer> getMerchantOffers(MerchantType merchantType);

    // Buy item from merchant
    bool buyItem(uint64_t accountId, MerchantType merchantType, const std::string& itemId,
                 int quantity, std::string& errorMsg);

    // Sell item to merchant
    bool sellItem(uint64_t accountId, MerchantType merchantType, uint32_t itemInstanceId,
                  std::string& errorMsg);

    // Get merchant by type
    Merchant* getMerchant(MerchantType type);

private:
    PersistenceManager* persistenceManager;
    std::map<MerchantType, Merchant> merchants;

    void initializeMerchants();
    void addOffer(Merchant& merchant, const std::string& itemId, int stock, float markup);
};
