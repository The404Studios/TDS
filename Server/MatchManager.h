#pragma once
#include "../Common/NetworkProtocol.h"
#include "../Common/DataStructures.h"
#include "../Common/ItemDatabase.h"
#include <map>
#include <vector>
#include <string>

// Match Manager - handles match creation, spawning, and raid management
class MatchManager {
public:
    MatchManager();

    // Create match from lobby
    bool createMatch(const std::vector<LobbyMember>& lobbyMembers, const std::string& mapName, uint64_t& outMatchId);

    // Get match
    Match* getMatch(uint64_t matchId);

    // Get player's match
    Match* getPlayerMatch(uint64_t accountId);

    // Update player position
    bool updatePlayerPosition(uint64_t accountId, float x, float y, float z, float yaw, float pitch);

    // Player takes damage
    bool playerTakeDamage(uint64_t accountId, float damage, uint64_t attackerId);

    // Player loots item
    bool playerLootItem(uint64_t accountId, uint64_t lootEntityId, Item& outItem);

    // Player extracts
    bool playerExtract(uint64_t accountId, const std::string& extractionName);

    // Update match (check for timeout)
    void update();

    // Get loot spawns for match
    const std::vector<LootSpawn>& getMatchLoot(uint64_t matchId);

    // Get AI enemies for match
    const std::vector<AIEnemy>& getMatchEnemies(uint64_t matchId);

    // Get extraction zones
    const std::vector<ExtractionZone>& getExtractionZones() const;

private:
    std::map<uint64_t, Match> matches;
    std::map<uint64_t, uint64_t> playerMatches;  // accountId -> matchId
    std::map<uint64_t, std::vector<LootSpawn>> matchLoot;
    std::map<uint64_t, std::vector<AIEnemy>> matchEnemies;
    std::vector<ExtractionZone> extractionZones;
    uint64_t nextMatchId;

    void generateSpawnPositions(Match& match);
    void generateLoot(Match& match);
    void spawnAIEnemies(Match& match);
    void initializeExtractionZones();
    void endMatch(uint64_t matchId);
};
