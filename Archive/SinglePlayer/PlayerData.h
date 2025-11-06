#pragma once
#include "InventorySystem.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Player's equipped items (what they bring into raid)
struct PlayerLoadout {
    std::shared_ptr<Item> primaryWeapon;
    std::shared_ptr<Item> secondaryWeapon;
    std::shared_ptr<Item> armor;
    std::shared_ptr<Item> helmet;
    std::shared_ptr<Item> backpack;

    Inventory pockets;           // 4x4 pocket space
    Inventory rig;               // 4x5 tactical rig/vest
    Inventory backpackStorage;   // Backpack storage (size depends on backpack)

    PlayerLoadout()
        : pockets("Pockets", 4, 4),
          rig("Tactical Rig", 4, 5),
          backpackStorage("Backpack", 6, 8) {
    }

    // Get total gear value
    int getTotalValue() {
        int total = 0;
        if (primaryWeapon) total += primaryWeapon->value;
        if (secondaryWeapon) total += secondaryWeapon->value;
        if (armor) total += armor->value;
        if (helmet) total += helmet->value;
        if (backpack) total += backpack->value;

        for (auto item : pockets.getAllItems()) total += item->value;
        for (auto item : rig.getAllItems()) total += item->value;
        for (auto item : backpackStorage.getAllItems()) total += item->value;

        return total;
    }

    // Clear all gear (on death without insurance)
    void clear() {
        primaryWeapon = nullptr;
        secondaryWeapon = nullptr;
        armor = nullptr;
        helmet = nullptr;
        backpack = nullptr;

        pockets = Inventory("Pockets", 4, 4);
        rig = Inventory("Tactical Rig", 4, 5);
        backpackStorage = Inventory("Backpack", 6, 8);
    }
};

// Persistent player data (survives between raids)
class PlayerData {
public:
    std::string playerName;
    int level;
    int experience;
    int roubles;                 // Money

    Inventory stash;             // Persistent stash (10x30 - standard edition size)
    PlayerLoadout loadout;       // Current equipped gear

    // Stats
    int raidsCompleted;
    int raidsExtracted;
    int raidsDied;
    int kills;
    int deaths;
    float survivalRate;

    PlayerData()
        : playerName("PMC"),
          level(1),
          experience(0),
          roubles(500000),           // Start with 500k roubles
          stash("Stash", 10, 30),    // 10x30 standard stash
          raidsCompleted(0),
          raidsExtracted(0),
          raidsDied(0),
          kills(0),
          deaths(0),
          survivalRate(0.0f) {
    }

    // Save to file
    bool save(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cout << "Failed to save player data to " << filename << std::endl;
            return false;
        }

        // Save basic data
        file << "PLAYERDATA_V1\n";
        file << playerName << "\n";
        file << level << "\n";
        file << experience << "\n";
        file << roubles << "\n";
        file << raidsCompleted << "\n";
        file << raidsExtracted << "\n";
        file << raidsDied << "\n";
        file << kills << "\n";
        file << deaths << "\n";

        // Save stash
        file << "STASH_BEGIN\n";
        saveInventory(file, stash);
        file << "STASH_END\n";

        // Save loadout
        file << "LOADOUT_BEGIN\n";
        saveLoadout(file);
        file << "LOADOUT_END\n";

        file.close();
        std::cout << "Player data saved successfully to " << filename << std::endl;
        return true;
    }

    // Load from file
    bool load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "No save file found at " << filename << ", using default player data\n";
            return false;
        }

        std::string line;
        std::getline(file, line); // Version header
        if (line != "PLAYERDATA_V1") {
            std::cout << "Invalid save file format\n";
            return false;
        }

        // Load basic data
        std::getline(file, playerName);
        file >> level >> experience >> roubles;
        file >> raidsCompleted >> raidsExtracted >> raidsDied;
        file >> kills >> deaths;
        file.ignore();

        // Calculate survival rate
        if (raidsCompleted > 0) {
            survivalRate = (float)raidsExtracted / raidsCompleted;
        }

        // Load stash
        std::getline(file, line); // STASH_BEGIN
        if (line == "STASH_BEGIN") {
            loadInventory(file, stash);
            std::getline(file, line); // STASH_END
        }

        // Load loadout
        std::getline(file, line); // LOADOUT_BEGIN
        if (line == "LOADOUT_BEGIN") {
            loadLoadout(file);
            std::getline(file, line); // LOADOUT_END
        }

        file.close();
        std::cout << "Player data loaded successfully from " << filename << std::endl;
        return true;
    }

    // Add items to stash after successful extraction
    void transferRaidLootToStash(std::vector<std::shared_ptr<Item>> loot) {
        int transferred = 0;
        int lost = 0;

        for (auto item : loot) {
            if (stash.addItem(item)) {
                transferred++;
            } else {
                lost++;
            }
        }

        std::cout << "Transferred " << transferred << " items to stash\n";
        if (lost > 0) {
            std::cout << "Lost " << lost << " items (stash full)\n";
        }
    }

    // Handle death - lose all gear unless insured (we'll skip insurance for now)
    void handleDeath() {
        deaths++;
        raidsDied++;
        loadout.clear();

        if (raidsCompleted > 0) {
            survivalRate = (float)raidsExtracted / raidsCompleted;
        }
    }

    // Handle extraction - keep all gear and loot
    void handleExtraction() {
        raidsCompleted++;
        raidsExtracted++;

        if (raidsCompleted > 0) {
            survivalRate = (float)raidsExtracted / raidsCompleted;
        }

        // Transfer raid inventory to stash
        std::vector<std::shared_ptr<Item>> allLoot;

        // Get items from pockets
        for (auto item : loadout.pockets.getAllItems()) {
            allLoot.push_back(item);
        }

        // Get items from rig
        for (auto item : loadout.rig.getAllItems()) {
            allLoot.push_back(item);
        }

        // Get items from backpack
        for (auto item : loadout.backpackStorage.getAllItems()) {
            allLoot.push_back(item);
        }

        transferRaidLootToStash(allLoot);
    }

    // Give starting gear
    void giveStartingGear() {
        auto& db = ItemDatabase::getInstance();

        // Add some starting cash and items to stash
        roubles = 500000;

        // Add starting gear to stash
        stash.addItem(db.createItem("ak74"));
        stash.addItem(db.createItem("glock17"));
        stash.addItem(db.createItem("paca"));
        stash.addItem(db.createItem("ssh68"));
        stash.addItem(db.createItem("scav"));

        // Add ammo
        stash.addItem(db.createItem("545x39"));
        stash.addItem(db.createItem("545x39"));
        stash.addItem(db.createItem("9x18"));

        // Add medical
        stash.addItem(db.createItem("ifak"));
        stash.addItem(db.createItem("ai2"));
        stash.addItem(db.createItem("ai2"));

        // Add food
        stash.addItem(db.createItem("water"));
        stash.addItem(db.createItem("tushonka"));
    }

private:
    void saveInventory(std::ofstream& file, Inventory& inv) {
        // Count unique items
        std::vector<std::shared_ptr<Item>> items = inv.getAllItems();
        file << items.size() << "\n";

        // Save each item
        for (auto item : items) {
            file << item->id << " ";
            file << item->stackSize << " ";
            file << item->foundInRaid << " ";
            file << item->currentAmmo << " ";
            file << item->durability << "\n";
        }
    }

    void loadInventory(std::ifstream& file, Inventory& inv) {
        int itemCount;
        file >> itemCount;
        file.ignore();

        auto& db = ItemDatabase::getInstance();

        for (int i = 0; i < itemCount; i++) {
            std::string id;
            int stackSize, fir, currentAmmo, durability;
            file >> id >> stackSize >> fir >> currentAmmo >> durability;
            file.ignore();

            auto item = db.createItem(id);
            if (item) {
                item->stackSize = stackSize;
                item->foundInRaid = (fir != 0);
                item->currentAmmo = currentAmmo;
                item->durability = durability;
                inv.addItem(item);
            }
        }
    }

    void saveLoadout(std::ofstream& file) {
        // Save equipped items (just IDs, or "NONE")
        file << (loadout.primaryWeapon ? loadout.primaryWeapon->id : "NONE") << "\n";
        file << (loadout.secondaryWeapon ? loadout.secondaryWeapon->id : "NONE") << "\n";
        file << (loadout.armor ? loadout.armor->id : "NONE") << "\n";
        file << (loadout.helmet ? loadout.helmet->id : "NONE") << "\n";
        file << (loadout.backpack ? loadout.backpack->id : "NONE") << "\n";

        // Save pocket inventory
        file << "POCKETS_BEGIN\n";
        saveInventory(file, loadout.pockets);
        file << "POCKETS_END\n";

        // Save rig inventory
        file << "RIG_BEGIN\n";
        saveInventory(file, loadout.rig);
        file << "RIG_END\n";

        // Save backpack inventory
        file << "BACKPACK_BEGIN\n";
        saveInventory(file, loadout.backpackStorage);
        file << "BACKPACK_END\n";
    }

    void loadLoadout(std::ifstream& file) {
        auto& db = ItemDatabase::getInstance();
        std::string line;

        // Load equipped items
        std::getline(file, line);
        loadout.primaryWeapon = (line != "NONE") ? db.createItem(line) : nullptr;

        std::getline(file, line);
        loadout.secondaryWeapon = (line != "NONE") ? db.createItem(line) : nullptr;

        std::getline(file, line);
        loadout.armor = (line != "NONE") ? db.createItem(line) : nullptr;

        std::getline(file, line);
        loadout.helmet = (line != "NONE") ? db.createItem(line) : nullptr;

        std::getline(file, line);
        loadout.backpack = (line != "NONE") ? db.createItem(line) : nullptr;

        // Load pockets
        std::getline(file, line); // POCKETS_BEGIN
        if (line == "POCKETS_BEGIN") {
            loadInventory(file, loadout.pockets);
            std::getline(file, line); // POCKETS_END
        }

        // Load rig
        std::getline(file, line); // RIG_BEGIN
        if (line == "RIG_BEGIN") {
            loadInventory(file, loadout.rig);
            std::getline(file, line); // RIG_END
        }

        // Load backpack
        std::getline(file, line); // BACKPACK_BEGIN
        if (line == "BACKPACK_BEGIN") {
            loadInventory(file, loadout.backpackStorage);
            std::getline(file, line); // BACKPACK_END
        }
    }
};
