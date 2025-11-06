#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include "../Common/ItemDatabase.h"
#include <map>
#include <fstream>
#include <string>

// Persistence Manager - handles saving and loading player data
class PersistenceManager {
public:
    PersistenceManager();
    ~PersistenceManager();

    // Create new player data for account
    bool createPlayerData(uint64_t accountId, const std::string& username);

    // Get player data
    PlayerData* getPlayerData(uint64_t accountId);

    // Save player data to file
    bool savePlayerData(uint64_t accountId);

    // Load player data from file
    bool loadPlayerData(uint64_t accountId);

    // Load all player data files
    void loadAllPlayerData();

    // Save all player data
    void saveAllPlayerData();

    // Handle post-raid (extraction)
    void handleExtraction(uint64_t accountId, const std::vector<Item>& lootCollected);

    // Handle post-raid (death)
    void handleDeath(uint64_t accountId);

    // Update kill stats
    void recordKill(uint64_t accountId);

private:
    std::map<uint64_t, PlayerData> playerDataMap;

    void saveItem(std::ofstream& file, const Item& item);
    bool loadItem(std::ifstream& file, Item& item);
    void initializeStartingGear(PlayerData& data);
};
