#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include "../Common/ItemDatabase.h"
#include <map>
#include <vector>
#include <random>
#include <cmath>

// Match Manager - handles match creation, spawning, and raid management
class MatchManager {
public:
    MatchManager() : nextMatchId(1) {
        initializeExtractionZones();
    }

    // Create match from lobby
    bool createMatch(const std::vector<LobbyMember>& lobbyMembers, const std::string& mapName, uint64_t& outMatchId) {
        if (lobbyMembers.empty()) {
            return false;
        }

        // Create match
        Match match;
        match.matchId = nextMatchId++;
        match.mapName = mapName;
        match.state = MatchState::STARTING;
        match.startTime = static_cast<float>(getCurrentTimestamp());
        match.raidDuration = 1800.0f;  // 30 minutes
        match.active = true;

        // Create players
        for (const auto& member : lobbyMembers) {
            MatchPlayer player;
            player.accountId = member.accountId;
            player.username = member.username;
            player.health = 440.0f;
            player.alive = true;
            player.extracted = false;
            match.players.push_back(player);

            // Map player to match
            playerMatches[member.accountId] = match.matchId;
        }

        // Generate spawn positions (party spawns together)
        generateSpawnPositions(match);

        // Generate loot
        generateLoot(match);

        // Spawn AI enemies
        spawnAIEnemies(match);

        matches[match.matchId] = match;
        outMatchId = match.matchId;

        // Set state to active
        matches[match.matchId].state = MatchState::ACTIVE;

        std::cout << "[MatchManager] Match created: " << match.matchId
                  << " (Map: " << mapName << ", Players: " << lobbyMembers.size() << ")" << std::endl;

        return true;
    }

    // Get match
    Match* getMatch(uint64_t matchId) {
        auto it = matches.find(matchId);
        if (it != matches.end()) {
            return &it->second;
        }
        return nullptr;
    }

    // Get player's match
    Match* getPlayerMatch(uint64_t accountId) {
        auto it = playerMatches.find(accountId);
        if (it != playerMatches.end()) {
            return getMatch(it->second);
        }
        return nullptr;
    }

    // Update player position
    bool updatePlayerPosition(uint64_t accountId, float x, float y, float z, float yaw, float pitch) {
        Match* match = getPlayerMatch(accountId);
        if (!match) return false;

        MatchPlayer* player = match->findPlayer(accountId);
        if (!player || !player->alive) return false;

        // Server-side validation: check for teleportation
        float distance = calculateDistance3D(player->x, player->y, player->z, x, y, z);
        if (distance > 50.0f) {
            std::cout << "[MatchManager] WARNING: Possible teleport detected for player " << accountId
                      << " (distance: " << distance << ")" << std::endl;
            // In production, this would trigger anti-cheat
            return false;
        }

        player->x = x;
        player->y = y;
        player->z = z;
        player->yaw = yaw;
        player->pitch = pitch;

        return true;
    }

    // Player takes damage
    bool playerTakeDamage(uint64_t accountId, float damage, uint64_t attackerId) {
        Match* match = getPlayerMatch(accountId);
        if (!match) return false;

        MatchPlayer* player = match->findPlayer(accountId);
        if (!player || !player->alive) return false;

        // Apply damage
        player->health -= damage;

        std::cout << "[MatchManager] Player " << accountId << " took " << damage
                  << " damage (HP: " << player->health << ")" << std::endl;

        // Check for death
        if (player->health <= 0) {
            player->alive = false;
            player->health = 0;

            std::cout << "[MatchManager] Player " << accountId << " died in match " << match->matchId << std::endl;

            // Check if match should end
            if (match->allExtractedOrDead()) {
                endMatch(match->matchId);
            }
        }

        return true;
    }

    // Player loots item
    bool playerLootItem(uint64_t accountId, uint64_t lootEntityId, Item& outItem) {
        Match* match = getPlayerMatch(accountId);
        if (!match) return false;

        MatchPlayer* player = match->findPlayer(accountId);
        if (!player || !player->alive) return false;

        // Find loot spawn
        for (auto& loot : matchLoot[match->matchId]) {
            if (loot.entityId == lootEntityId && !loot.collected) {
                // Validate proximity (must be within 5 meters)
                float distance = calculateDistance3D(player->x, player->y, player->z, loot.x, loot.y, loot.z);
                if (distance > 5.0f) {
                    std::cout << "[MatchManager] WARNING: Loot too far for player " << accountId
                              << " (distance: " << distance << ")" << std::endl;
                    return false;
                }

                // Mark as collected
                loot.collected = true;

                // Add to player's collected loot
                outItem = loot.item;
                outItem.foundInRaid = true;
                player->lootCollected.push_back(outItem);

                std::cout << "[MatchManager] Player " << accountId << " looted " << loot.item.name << std::endl;

                return true;
            }
        }

        return false;
    }

    // Player extracts
    bool playerExtract(uint64_t accountId, const std::string& extractionName) {
        Match* match = getPlayerMatch(accountId);
        if (!match) return false;

        MatchPlayer* player = match->findPlayer(accountId);
        if (!player || !player->alive || player->extracted) return false;

        // Find extraction zone
        for (const auto& zone : extractionZones) {
            if (zone.name == extractionName && zone.active) {
                // Validate player is in zone
                float distance = calculateDistance2D(player->x, player->z, zone.x, zone.z);
                if (distance > zone.radius) {
                    std::cout << "[MatchManager] Player " << accountId << " not in extraction zone" << std::endl;
                    return false;
                }

                // Extract player
                player->extracted = true;
                playerMatches.erase(accountId);

                std::cout << "[MatchManager] Player " << accountId << " extracted from match " << match->matchId << std::endl;

                // Check if match should end
                if (match->allExtractedOrDead()) {
                    endMatch(match->matchId);
                }

                return true;
            }
        }

        return false;
    }

    // Update match (check for timeout)
    void update() {
        uint64_t currentTime = getCurrentTimestamp();

        std::vector<uint64_t> toEnd;

        for (auto& pair : matches) {
            Match& match = pair.second;

            if (match.state == MatchState::ACTIVE) {
                // Check for timeout
                float elapsed = static_cast<float>(currentTime) - match.startTime;
                if (elapsed >= match.raidDuration) {
                    std::cout << "[MatchManager] Match " << match.matchId << " timed out" << std::endl;
                    toEnd.push_back(match.matchId);
                }

                // Check if all players extracted or died
                if (match.allExtractedOrDead()) {
                    toEnd.push_back(match.matchId);
                }
            }
        }

        // End timed-out matches
        for (uint64_t matchId : toEnd) {
            endMatch(matchId);
        }
    }

    // Get loot spawns for match
    const std::vector<LootSpawn>& getMatchLoot(uint64_t matchId) {
        return matchLoot[matchId];
    }

    // Get AI enemies for match
    const std::vector<AIEnemy>& getMatchEnemies(uint64_t matchId) {
        return matchEnemies[matchId];
    }

    // Get extraction zones
    const std::vector<ExtractionZone>& getExtractionZones() const {
        return extractionZones;
    }

private:
    std::map<uint64_t, Match> matches;
    std::map<uint64_t, uint64_t> playerMatches;  // accountId -> matchId
    std::map<uint64_t, std::vector<LootSpawn>> matchLoot;
    std::map<uint64_t, std::vector<AIEnemy>> matchEnemies;
    std::vector<ExtractionZone> extractionZones;
    uint64_t nextMatchId;

    void generateSpawnPositions(Match& match) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> dis(-100.0f, 100.0f);

        // Choose random spawn point
        float spawnX = dis(gen);
        float spawnZ = dis(gen);
        float spawnY = 10.0f;

        // Spawn all party members within 50m of each other
        for (size_t i = 0; i < match.players.size(); i++) {
            std::uniform_real_distribution<float> offset(-25.0f, 25.0f);
            match.players[i].x = spawnX + offset(gen);
            match.players[i].y = spawnY;
            match.players[i].z = spawnZ + offset(gen);
            match.players[i].yaw = dis(gen);
            match.players[i].pitch = 0.0f;

            std::cout << "[MatchManager] Player " << match.players[i].username
                      << " spawned at (" << match.players[i].x << ", "
                      << match.players[i].y << ", " << match.players[i].z << ")" << std::endl;
        }
    }

    void generateLoot(Match& match) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> posDis(-150.0f, 150.0f);
        static std::uniform_int_distribution<int> countDis(30, 60);

        auto& itemDb = ItemDatabase::getInstance();
        auto allItemIds = itemDb.getAllItemIds();

        std::vector<LootSpawn> loot;
        int lootCount = countDis(gen);

        for (int i = 0; i < lootCount; i++) {
            LootSpawn spawn;
            spawn.entityId = match.matchId * 10000 + i;

            // Random position
            spawn.x = posDis(gen);
            spawn.y = 0.5f;
            spawn.z = posDis(gen);

            // Random item
            std::uniform_int_distribution<size_t> itemDis(0, allItemIds.size() - 1);
            std::string itemId = allItemIds[itemDis(gen)];
            spawn.item = itemDb.createItem(itemId, spawn.entityId);

            spawn.collected = false;

            loot.push_back(spawn);
        }

        matchLoot[match.matchId] = loot;
        std::cout << "[MatchManager] Generated " << lootCount << " loot spawns for match " << match.matchId << std::endl;
    }

    void spawnAIEnemies(Match& match) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<float> posDis(-150.0f, 150.0f);
        static std::uniform_int_distribution<int> countDis(8, 15);

        std::vector<AIEnemy> enemies;
        int enemyCount = countDis(gen);

        for (int i = 0; i < enemyCount; i++) {
            AIEnemy enemy;
            enemy.entityId = match.matchId * 10000 + 1000 + i;
            enemy.type = AIType::SCAV;
            enemy.x = posDis(gen);
            enemy.y = 1.0f;
            enemy.z = posDis(gen);
            enemy.yaw = posDis(gen);
            enemy.health = 100.0f;
            enemy.maxHealth = 100.0f;
            enemy.alive = true;
            enemy.aggroed = false;
            enemy.targetPlayerId = 0;

            enemies.push_back(enemy);
        }

        matchEnemies[match.matchId] = enemies;
        std::cout << "[MatchManager] Spawned " << enemyCount << " AI enemies for match " << match.matchId << std::endl;
    }

    void initializeExtractionZones() {
        ExtractionZone zone1;
        zone1.name = "Railroad Bridge";
        zone1.x = -120.0f;
        zone1.y = 0.0f;
        zone1.z = -120.0f;
        zone1.radius = 10.0f;
        zone1.extractTime = 8.0f;
        zone1.requiresKey = false;
        zone1.active = true;

        ExtractionZone zone2;
        zone2.name = "Scav Checkpoint";
        zone2.x = 130.0f;
        zone2.y = 0.0f;
        zone2.z = 130.0f;
        zone2.radius = 12.0f;
        zone2.extractTime = 10.0f;
        zone2.requiresKey = false;
        zone2.active = true;

        ExtractionZone zone3;
        zone3.name = "Boat Dock";
        zone3.x = -100.0f;
        zone3.y = 0.0f;
        zone3.z = 140.0f;
        zone3.radius = 8.0f;
        zone3.extractTime = 7.0f;
        zone3.requiresKey = false;
        zone3.active = true;

        extractionZones.push_back(zone1);
        extractionZones.push_back(zone2);
        extractionZones.push_back(zone3);
    }

    void endMatch(uint64_t matchId) {
        auto it = matches.find(matchId);
        if (it == matches.end()) return;

        Match& match = it->second;
        match.state = MatchState::ENDING;
        match.active = false;

        std::cout << "[MatchManager] Match " << matchId << " ended" << std::endl;

        // Remove player mappings
        for (const auto& player : match.players) {
            playerMatches.erase(player.accountId);
        }

        // Clean up loot and enemies
        matchLoot.erase(matchId);
        matchEnemies.erase(matchId);

        // Mark as finished
        match.state = MatchState::FINISHED;
    }
};
