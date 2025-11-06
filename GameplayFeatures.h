#ifndef GAMEPLAY_FEATURES_H
#define GAMEPLAY_FEATURES_H

#include "CivilizationAI.h"
#include <map>
#include <set>

// Diplomacy system
enum class DiplomaticRelation {
    WAR,
    NEUTRAL,
    TRADE_AGREEMENT,
    NON_AGGRESSION_PACT,
    ALLIANCE
};

enum class DiplomaticAction {
    DECLARE_WAR,
    OFFER_PEACE,
    PROPOSE_TRADE,
    PROPOSE_ALLIANCE,
    BREAK_ALLIANCE,
    DEMAND_TRIBUTE,
    OFFER_GIFT
};

struct DiplomaticState {
    DiplomaticRelation relation;
    float trustLevel; // 0-100
    float tradeBalance;
    int turnsAtWar;
    int turnsAtPeace;

    DiplomaticState() : relation(DiplomaticRelation::NEUTRAL),
                        trustLevel(50.0f), tradeBalance(0),
                        turnsAtWar(0), turnsAtPeace(0) {}
};

// Technology system
enum class TechnologyType {
    // Military technologies
    BRONZE_WEAPONS,
    IRON_WEAPONS,
    STEEL_WEAPONS,
    ARCHERY,
    CAVALRY,
    SIEGE_WEAPONS,

    // Economic technologies
    AGRICULTURE,
    IRRIGATION,
    MINING,
    METALLURGY,
    CURRENCY,
    BANKING,

    // Civic technologies
    WRITING,
    PHILOSOPHY,
    MATHEMATICS,
    ENGINEERING,
    MEDICINE,
    EDUCATION,

    // Defensive technologies
    MASONRY,
    CONSTRUCTION,
    FORTIFICATIONS,
    WALLS,

    COUNT
};

struct Technology {
    TechnologyType type;
    std::string name;
    std::string description;
    int researchCost;
    std::vector<TechnologyType> prerequisites;
    bool researched;

    // Effects
    float militaryBonus;
    float economicBonus;
    float healthBonus;
    float buildSpeedBonus;

    Technology() : researched(false), militaryBonus(0), economicBonus(0),
                   healthBonus(0), buildSpeedBonus(0) {}
};

// Trading system
enum class TradeResourceType {
    FOOD,
    WOOD,
    STONE,
    GOLD,
    WEAPONS,
    LUXURY_GOODS
};

struct TradeOffer {
    Faction offeringFaction;
    Faction receivingFaction;
    std::map<TradeResourceType, int> offeredResources;
    std::map<TradeResourceType, int> requestedResources;
    int goldOffered;
    int goldRequested;
    bool accepted;
    float expirationTime;

    TradeOffer() : accepted(false), expirationTime(0) {}
};

// Weather system
enum class WeatherType {
    CLEAR,
    RAIN,
    STORM,
    SNOW,
    FOG,
    DROUGHT
};

struct WeatherEffect {
    WeatherType type;
    float intensity; // 0-1
    float duration;
    float timeRemaining;

    // Gameplay effects
    float movementSpeedModifier;
    float visibilityModifier;
    float farmProductivityModifier;
    float combatEffectivenessModifier;

    WeatherEffect() : type(WeatherType::CLEAR), intensity(0), duration(0),
                      timeRemaining(0), movementSpeedModifier(1.0f),
                      visibilityModifier(1.0f), farmProductivityModifier(1.0f),
                      combatEffectivenessModifier(1.0f) {}
};

// Advanced combat system
enum class CombatFormation {
    SCATTERED,
    LINE,
    COLUMN,
    CIRCLE,
    WEDGE
};

struct CombatUnit {
    std::vector<Agent*> agents;
    CombatFormation formation;
    Agent* commander;
    Vector3 targetPosition;
    float morale; // 0-100
    float experience; // Increases with battles

    CombatUnit() : commander(nullptr), morale(75.0f), experience(0) {}

    float getCombatEffectiveness() const {
        return (morale / 100.0f) * (1.0f + experience * 0.01f);
    }
};

// Quest/Mission system
enum class MissionType {
    DESTROY_BUILDING,
    KILL_AGENTS,
    COLLECT_RESOURCES,
    DEFEND_LOCATION,
    EXPLORE_AREA,
    ESTABLISH_TRADE_ROUTE
};

struct Mission {
    MissionType type;
    Faction assignedFaction;
    std::string description;
    Vector3 location;
    int targetAmount;
    int currentProgress;
    float timeLimit;
    float timeElapsed;
    bool completed;

    // Rewards
    std::map<ResourceType, int> resourceRewards;
    int experienceReward;

    Mission() : targetAmount(0), currentProgress(0), timeLimit(0),
                timeElapsed(0), completed(false), experienceReward(0) {}
};

// Population growth and happiness system
struct PopulationStats {
    int population;
    float birthRate;
    float deathRate;
    float immigrationRate;
    float emigrationRate;
    float happiness; // 0-100
    float literacy;
    float health;

    PopulationStats() : population(0), birthRate(0.02f), deathRate(0.01f),
                        immigrationRate(0.005f), emigrationRate(0.002f),
                        happiness(50.0f), literacy(20.0f), health(60.0f) {}

    float getGrowthRate() const {
        return birthRate - deathRate + immigrationRate - emigrationRate;
    }
};

// Advanced faction manager with new features
class AdvancedFactionManager : public FactionManager {
public:
    // Diplomacy
    std::map<Faction, DiplomaticState> diplomaticRelations;

    // Technology
    std::map<TechnologyType, Technology> technologies;
    TechnologyType currentResearch;
    int researchPoints;

    // Trading
    std::vector<TradeOffer> activeTradeOffers;

    // Combat
    std::vector<CombatUnit> militaryUnits;

    // Missions
    std::vector<Mission> activeMissions;

    // Population
    PopulationStats populationStats;

    // Advanced policies
    float educationFunding; // 0-1
    float healthcareFunding;
    float infrastructureFunding;
    float scienceFunding;

    AdvancedFactionManager(Faction f);

    // Diplomacy functions
    void performDiplomaticAction(Faction targetFaction, DiplomaticAction action);
    bool isAtWarWith(Faction other) const;
    bool hasAllianceWith(Faction other) const;
    void updateDiplomacy(float deltaTime);

    // Technology functions
    void startResearch(TechnologyType tech);
    void updateResearch(float deltaTime);
    bool hasTechnology(TechnologyType tech) const;
    float getTechnologyBonus(const std::string& category) const;

    // Trading functions
    void proposeTrade(Faction targetFaction, const TradeOffer& offer);
    void acceptTrade(const TradeOffer& offer);
    void rejectTrade(const TradeOffer& offer);
    void updateTrades(float deltaTime);

    // Combat functions
    void formCombatUnit(const std::vector<Agent*>& agents, CombatFormation formation);
    void updateCombatUnits(float deltaTime);
    void issueCombatOrder(CombatUnit& unit, const Vector3& target);

    // Mission functions
    void assignMission(const Mission& mission);
    void updateMissions(float deltaTime);
    void completeMission(Mission& mission);

    // Population functions
    void updatePopulation(float deltaTime);
    void calculateHappiness();

    // Advanced update
    void advancedUpdate(float deltaTime);
};

// Global game manager for advanced features
class AdvancedGameManager {
public:
    // Weather system
    WeatherEffect currentWeather;
    float weatherChangeTimer;

    // Global events
    struct GlobalEvent {
        std::string name;
        std::string description;
        float duration;
        std::function<void(World*)> effect;
    };
    std::vector<GlobalEvent> activeEvents;

    // Season system
    enum class Season {
        SPRING,
        SUMMER,
        FALL,
        WINTER
    };
    Season currentSeason;
    float seasonTimer;

    AdvancedGameManager();

    // Weather functions
    void updateWeather(float deltaTime);
    void changeWeather(WeatherType newWeather);
    WeatherEffect& getCurrentWeather() { return currentWeather; }

    // Season functions
    void updateSeason(float deltaTime);
    Season getCurrentSeason() const { return currentSeason; }
    float getSeasonModifier(const std::string& category) const;

    // Event functions
    void triggerEvent(const GlobalEvent& event);
    void updateEvents(float deltaTime, World* world);

    // Victory conditions
    bool checkVictoryConditions(const World* world, Faction& winner);

    void update(float deltaTime, World* world);
};

// Resource distribution and market system
class MarketSystem {
public:
    // Resource prices (dynamic based on supply/demand)
    std::map<TradeResourceType, float> prices;

    // Supply and demand
    std::map<TradeResourceType, int> globalSupply;
    std::map<TradeResourceType, int> globalDemand;

    MarketSystem();

    void updatePrices(float deltaTime);
    float getPrice(TradeResourceType resource) const;
    void addSupply(TradeResourceType resource, int amount);
    void addDemand(TradeResourceType resource, int amount);
};

// Initialize all technologies
void initializeTechnologies(std::map<TechnologyType, Technology>& techTree);

// Weather effect presets
WeatherEffect createWeatherEffect(WeatherType type);

#endif // GAMEPLAY_FEATURES_H
