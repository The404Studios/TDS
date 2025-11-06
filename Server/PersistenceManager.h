#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include "../Common/ItemDatabase.h"
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>

// Persistence Manager - handles saving and loading player data
class PersistenceManager {
public:
    PersistenceManager() {
        loadAllPlayerData();
    }

    ~PersistenceManager() {
        saveAllPlayerData();
    }

    // Create new player data for account
    bool createPlayerData(uint64_t accountId, const std::string& username) {
        // Check if already exists
        if (playerDataMap.find(accountId) != playerDataMap.end()) {
            std::cout << "[PersistenceManager] Player data already exists for account " << accountId << std::endl;
            return false;
        }

        // Create new player data
        PlayerData playerData;
        playerData.accountId = accountId;
        playerData.username = username;

        // Initialize with starting gear
        initializeStartingGear(playerData);

        playerDataMap[accountId] = playerData;

        std::cout << "[PersistenceManager] Created player data for " << username << std::endl;

        // Auto-save
        savePlayerData(accountId);

        return true;
    }

    // Get player data
    PlayerData* getPlayerData(uint64_t accountId) {
        auto it = playerDataMap.find(accountId);
        if (it != playerDataMap.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Save player data to file
    bool savePlayerData(uint64_t accountId) {
        auto it = playerDataMap.find(accountId);
        if (it == playerDataMap.end()) {
            std::cout << "[PersistenceManager] Cannot save - player data not found for account " << accountId << std::endl;
            return false;
        }

        PlayerData& data = it->second;

        std::string filename = "Server/playerdata_" + std::to_string(accountId) + ".dat";
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "[PersistenceManager] Failed to open file for writing: " << filename << std::endl;
            return false;
        }

        // Write header
        file << "PLAYERDATA_V1\n";

        // Write basic info
        file << data.accountId << "\n";
        file << data.username << "\n";

        // Write stats
        file << data.stats.level << "\n";
        file << data.stats.experience << "\n";
        file << data.stats.roubles << "\n";
        file << data.stats.raidsCompleted << "\n";
        file << data.stats.raidsExtracted << "\n";
        file << data.stats.raidsDied << "\n";
        file << data.stats.kills << "\n";
        file << data.stats.deaths << "\n";

        // Write stash
        file << "STASH_BEGIN\n";
        file << data.stash.size() << "\n";
        for (const auto& item : data.stash) {
            saveItem(file, item);
        }
        file << "STASH_END\n";

        // Write loadout
        file << "LOADOUT_BEGIN\n";
        file << data.loadout.size() << "\n";
        for (const auto& item : data.loadout) {
            saveItem(file, item);
        }
        file << "LOADOUT_END\n";

        file.close();
        return true;
    }

    // Load player data from file
    bool loadPlayerData(uint64_t accountId) {
        std::string filename = "Server/playerdata_" + std::to_string(accountId) + ".dat";
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "[PersistenceManager] No save file found for account " << accountId << std::endl;
            return false;
        }

        PlayerData data;

        std::string line;
        std::getline(file, line);  // Version header
        if (line != "PLAYERDATA_V1") {
            std::cout << "[PersistenceManager] Invalid save file version" << std::endl;
            return false;
        }

        // Read basic info
        file >> data.accountId;
        file.ignore();
        std::getline(file, data.username);

        // Read stats
        file >> data.stats.level;
        file >> data.stats.experience;
        file >> data.stats.roubles;
        file >> data.stats.raidsCompleted;
        file >> data.stats.raidsExtracted;
        file >> data.stats.raidsDied;
        file >> data.stats.kills;
        file >> data.stats.deaths;
        file.ignore();

        // Calculate survival rate
        if (data.stats.raidsCompleted > 0) {
            data.stats.survivalRate = static_cast<float>(data.stats.raidsExtracted) / data.stats.raidsCompleted;
        }

        // Read stash
        std::getline(file, line);  // STASH_BEGIN
        if (line == "STASH_BEGIN") {
            int itemCount;
            file >> itemCount;
            file.ignore();

            for (int i = 0; i < itemCount; i++) {
                Item item;
                if (loadItem(file, item)) {
                    data.stash.push_back(item);
                }
            }

            std::getline(file, line);  // STASH_END
        }

        // Read loadout
        std::getline(file, line);  // LOADOUT_BEGIN
        if (line == "LOADOUT_BEGIN") {
            int itemCount;
            file >> itemCount;
            file.ignore();

            for (int i = 0; i < itemCount; i++) {
                Item item;
                if (loadItem(file, item)) {
                    data.loadout.push_back(item);
                }
            }

            std::getline(file, line);  // LOADOUT_END
        }

        file.close();

        playerDataMap[accountId] = data;
        std::cout << "[PersistenceManager] Loaded player data for " << data.username << std::endl;

        return true;
    }

    // Load all player data files
    void loadAllPlayerData() {
        // In a real implementation, you would scan the directory for all playerdata_*.dat files
        // For now, player data is loaded on-demand
        std::cout << "[PersistenceManager] Persistence manager initialized" << std::endl;
    }

    // Save all player data
    void saveAllPlayerData() {
        int count = 0;
        for (const auto& pair : playerDataMap) {
            if (savePlayerData(pair.first)) {
                count++;
            }
        }
        std::cout << "[PersistenceManager] Saved " << count << " player profiles" << std::endl;
    }

    // Handle post-raid (extraction)
    void handleExtraction(uint64_t accountId, const std::vector<Item>& lootCollected) {
        PlayerData* data = getPlayerData(accountId);
        if (!data) return;

        // Update stats
        data->stats.raidsCompleted++;
        data->stats.raidsExtracted++;

        if (data->stats.raidsCompleted > 0) {
            data->stats.survivalRate = static_cast<float>(data->stats.raidsExtracted) / data->stats.raidsCompleted;
        }

        // Transfer loot to stash
        for (const auto& item : lootCollected) {
            data->stash.push_back(item);
        }

        // Keep loadout intact

        std::cout << "[PersistenceManager] Player " << accountId << " extracted with " << lootCollected.size() << " items" << std::endl;

        savePlayerData(accountId);
    }

    // Handle post-raid (death)
    void handleDeath(uint64_t accountId) {
        PlayerData* data = getPlayerData(accountId);
        if (!data) return;

        // Update stats
        data->stats.raidsCompleted++;
        data->stats.raidsDied++;
        data->stats.deaths++;

        if (data->stats.raidsCompleted > 0) {
            data->stats.survivalRate = static_cast<float>(data->stats.raidsExtracted) / data->stats.raidsCompleted;
        }

        // Lose all loadout (no insurance system yet)
        data->loadout.clear();

        std::cout << "[PersistenceManager] Player " << accountId << " died and lost all gear" << std::endl;

        savePlayerData(accountId);
    }

    // Update kill stats
    void recordKill(uint64_t accountId) {
        PlayerData* data = getPlayerData(accountId);
        if (!data) return;

        data->stats.kills++;
        savePlayerData(accountId);
    }

private:
    std::map<uint64_t, PlayerData> playerDataMap;

    void saveItem(std::ofstream& file, const Item& item) {
        file << item.instanceId << "\n";
        file << item.id << "\n";
        file << item.stackSize << "\n";
        file << (item.foundInRaid ? 1 : 0) << "\n";
        file << item.currentAmmo << "\n";
        file << item.durability << "\n";
    }

    bool loadItem(std::ifstream& file, Item& item) {
        auto& itemDb = ItemDatabase::getInstance();

        uint32_t instanceId;
        std::string itemId;
        int stackSize, fir, currentAmmo, durability;

        file >> instanceId;
        file.ignore();
        std::getline(file, itemId);
        file >> stackSize >> fir >> currentAmmo >> durability;
        file.ignore();

        // Create item from template
        item = itemDb.createItem(itemId, instanceId);
        if (item.id.empty()) {
            std::cout << "[PersistenceManager] WARNING: Unknown item ID: " << itemId << std::endl;
            return false;
        }

        // Restore saved properties
        item.stackSize = stackSize;
        item.foundInRaid = (fir != 0);
        item.currentAmmo = currentAmmo;
        item.durability = durability;

        return true;
    }

    void initializeStartingGear(PlayerData& data) {
        auto& itemDb = ItemDatabase::getInstance();

        // Starting money
        data.stats.roubles = 500000;

        // Add starting weapons to stash
        data.stash.push_back(itemDb.createItem("ak74", 1));
        data.stash.push_back(itemDb.createItem("glock17", 2));

        // Add starting armor
        data.stash.push_back(itemDb.createItem("paca", 3));
        data.stash.push_back(itemDb.createItem("ssh68", 4));
        data.stash.push_back(itemDb.createItem("scav", 5));

        // Add ammo
        data.stash.push_back(itemDb.createItem("545x39", 6));
        data.stash.push_back(itemDb.createItem("545x39", 7));
        data.stash.push_back(itemDb.createItem("9x18", 8));

        // Add medical
        data.stash.push_back(itemDb.createItem("ifak", 9));
        data.stash.push_back(itemDb.createItem("ai2", 10));
        data.stash.push_back(itemDb.createItem("ai2", 11));

        // Add food
        data.stash.push_back(itemDb.createItem("water", 12));
        data.stash.push_back(itemDb.createItem("tushonka", 13));

        std::cout << "[PersistenceManager] Initialized starting gear for " << data.username << std::endl;
    }
};
