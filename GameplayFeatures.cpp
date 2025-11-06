#include "GameplayFeatures.h"
#include <algorithm>
#include <cmath>
#include <iostream>

// AdvancedFactionManager implementation

AdvancedFactionManager::AdvancedFactionManager(Faction f)
    : FactionManager(f), researchPoints(0), educationFunding(0.2f),
      healthcareFunding(0.2f), infrastructureFunding(0.3f), scienceFunding(0.3f) {

    // Initialize diplomatic relations with other factions
    for (int i = 0; i < static_cast<int>(Faction::COUNT); ++i) {
        if (static_cast<Faction>(i) != faction) {
            diplomaticRelations[static_cast<Faction>(i)] = DiplomaticState();
        }
    }

    // Initialize technology tree
    initializeTechnologies(technologies);
}

void AdvancedFactionManager::performDiplomaticAction(Faction targetFaction, DiplomaticAction action) {
    if (targetFaction == faction) return;

    DiplomaticState& state = diplomaticRelations[targetFaction];

    switch (action) {
        case DiplomaticAction::DECLARE_WAR:
            state.relation = DiplomaticRelation::WAR;
            state.turnsAtWar = 0;
            state.trustLevel = std::max(0.0f, state.trustLevel - 50.0f);
            std::cout << "Faction " << static_cast<int>(faction) << " declares war on "
                      << static_cast<int>(targetFaction) << "!" << std::endl;
            break;

        case DiplomaticAction::OFFER_PEACE:
            if (state.relation == DiplomaticRelation::WAR) {
                state.relation = DiplomaticRelation::NEUTRAL;
                state.turnsAtPeace = 0;
                std::cout << "Faction " << static_cast<int>(faction) << " offers peace to "
                          << static_cast<int>(targetFaction) << std::endl;
            }
            break;

        case DiplomaticAction::PROPOSE_ALLIANCE:
            if (state.trustLevel > 60.0f && state.relation != DiplomaticRelation::WAR) {
                state.relation = DiplomaticRelation::ALLIANCE;
                state.trustLevel = std::min(100.0f, state.trustLevel + 20.0f);
                std::cout << "Alliance formed between " << static_cast<int>(faction)
                          << " and " << static_cast<int>(targetFaction) << "!" << std::endl;
            }
            break;

        case DiplomaticAction::BREAK_ALLIANCE:
            if (state.relation == DiplomaticRelation::ALLIANCE) {
                state.relation = DiplomaticRelation::NEUTRAL;
                state.trustLevel = std::max(0.0f, state.trustLevel - 40.0f);
            }
            break;

        case DiplomaticAction::PROPOSE_TRADE:
            if (state.relation != DiplomaticRelation::WAR) {
                state.relation = DiplomaticRelation::TRADE_AGREEMENT;
                state.trustLevel = std::min(100.0f, state.trustLevel + 5.0f);
            }
            break;

        case DiplomaticAction::OFFER_GIFT:
            state.trustLevel = std::min(100.0f, state.trustLevel + 10.0f);
            break;

        case DiplomaticAction::DEMAND_TRIBUTE:
            state.trustLevel = std::max(0.0f, state.trustLevel - 15.0f);
            break;
    }
}

bool AdvancedFactionManager::isAtWarWith(Faction other) const {
    auto it = diplomaticRelations.find(other);
    return it != diplomaticRelations.end() && it->second.relation == DiplomaticRelation::WAR;
}

bool AdvancedFactionManager::hasAllianceWith(Faction other) const {
    auto it = diplomaticRelations.find(other);
    return it != diplomaticRelations.end() && it->second.relation == DiplomaticRelation::ALLIANCE;
}

void AdvancedFactionManager::updateDiplomacy(float deltaTime) {
    for (auto& pair : diplomaticRelations) {
        DiplomaticState& state = pair.second;

        if (state.relation == DiplomaticRelation::WAR) {
            state.turnsAtWar++;
        } else {
            state.turnsAtPeace++;
        }

        // Trust slowly decays or grows toward neutral
        if (state.trustLevel < 50.0f) {
            state.trustLevel += deltaTime * 0.1f;
        } else if (state.trustLevel > 50.0f) {
            state.trustLevel -= deltaTime * 0.05f;
        }
    }
}

void AdvancedFactionManager::startResearch(TechnologyType tech) {
    auto it = technologies.find(tech);
    if (it != technologies.end() && !it->second.researched) {
        currentResearch = tech;
        std::cout << "Faction " << static_cast<int>(faction)
                  << " begins researching " << it->second.name << std::endl;
    }
}

void AdvancedFactionManager::updateResearch(float deltaTime) {
    auto it = technologies.find(currentResearch);
    if (it == technologies.end() || it->second.researched) {
        return;
    }

    Technology& tech = it->second;

    // Add research points based on science funding
    researchPoints += static_cast<int>(scienceFunding * 10.0f * deltaTime);

    if (researchPoints >= tech.researchCost) {
        tech.researched = true;
        researchPoints = 0;
        std::cout << "Faction " << static_cast<int>(faction)
                  << " completed research: " << tech.name << "!" << std::endl;
    }
}

bool AdvancedFactionManager::hasTechnology(TechnologyType tech) const {
    auto it = technologies.find(tech);
    return it != technologies.end() && it->second.researched;
}

float AdvancedFactionManager::getTechnologyBonus(const std::string& category) const {
    float bonus = 1.0f;

    for (const auto& pair : technologies) {
        if (pair.second.researched) {
            if (category == "military") {
                bonus += pair.second.militaryBonus;
            } else if (category == "economic") {
                bonus += pair.second.economicBonus;
            } else if (category == "health") {
                bonus += pair.second.healthBonus;
            } else if (category == "build") {
                bonus += pair.second.buildSpeedBonus;
            }
        }
    }

    return bonus;
}

void AdvancedFactionManager::proposeTrade(Faction targetFaction, const TradeOffer& offer) {
    TradeOffer newOffer = offer;
    newOffer.offeringFaction = faction;
    newOffer.receivingFaction = targetFaction;
    newOffer.expirationTime = 30.0f; // 30 seconds to accept
    activeTradeOffers.push_back(newOffer);
}

void AdvancedFactionManager::acceptTrade(const TradeOffer& offer) {
    // Execute trade
    for (const auto& pair : offer.offeredResources) {
        int index = static_cast<int>(pair.first);
        if (index < resources.size()) {
            resources[index] += pair.second;
        }
    }

    for (const auto& pair : offer.requestedResources) {
        int index = static_cast<int>(pair.first);
        if (index < resources.size()) {
            resources[index] -= pair.second;
        }
    }

    // Update diplomatic relations
    auto& state = diplomaticRelations[offer.offeringFaction];
    state.trustLevel = std::min(100.0f, state.trustLevel + 5.0f);
    state.tradeBalance += offer.goldOffered - offer.goldRequested;
}

void AdvancedFactionManager::rejectTrade(const TradeOffer& offer) {
    auto& state = diplomaticRelations[offer.offeringFaction];
    state.trustLevel = std::max(0.0f, state.trustLevel - 2.0f);
}

void AdvancedFactionManager::updateTrades(float deltaTime) {
    for (auto it = activeTradeOffers.begin(); it != activeTradeOffers.end();) {
        it->expirationTime -= deltaTime;
        if (it->expirationTime <= 0 || it->accepted) {
            it = activeTradeOffers.erase(it);
        } else {
            ++it;
        }
    }
}

void AdvancedFactionManager::formCombatUnit(const std::vector<Agent*>& agents, CombatFormation formation) {
    CombatUnit unit;
    unit.agents = agents;
    unit.formation = formation;

    // Find highest ranking soldier as commander
    for (Agent* agent : agents) {
        if (agent->role == Role::SOLDIER) {
            if (!unit.commander || agent->kills > unit.commander->kills) {
                unit.commander = agent;
            }
        }
    }

    militaryUnits.push_back(unit);
}

void AdvancedFactionManager::updateCombatUnits(float deltaTime) {
    for (auto& unit : militaryUnits) {
        // Remove dead agents
        unit.agents.erase(
            std::remove_if(unit.agents.begin(), unit.agents.end(),
                [](Agent* a) { return !a->isAlive(); }),
            unit.agents.end()
        );

        // Update morale based on unit strength and health
        if (!unit.agents.empty()) {
            float avgHealth = 0;
            for (Agent* agent : unit.agents) {
                avgHealth += agent->health;
            }
            avgHealth /= unit.agents.size();

            unit.morale = avgHealth * (unit.agents.size() / 10.0f);
            unit.morale = std::min(100.0f, std::max(0.0f, unit.morale));
        }
    }

    // Remove empty units
    militaryUnits.erase(
        std::remove_if(militaryUnits.begin(), militaryUnits.end(),
            [](const CombatUnit& u) { return u.agents.empty(); }),
        militaryUnits.end()
    );
}

void AdvancedFactionManager::issueCombatOrder(CombatUnit& unit, const Vector3& target) {
    unit.targetPosition = target;

    // Set target for all agents in unit
    for (Agent* agent : unit.agents) {
        agent->targetPosition = target;
    }
}

void AdvancedFactionManager::assignMission(const Mission& mission) {
    activeMissions.push_back(mission);
}

void AdvancedFactionManager::updateMissions(float deltaTime) {
    for (auto& mission : activeMissions) {
        if (!mission.completed) {
            mission.timeElapsed += deltaTime;

            if (mission.timeElapsed >= mission.timeLimit && mission.timeLimit > 0) {
                // Mission failed
                std::cout << "Mission failed: " << mission.description << std::endl;
                mission.completed = true;
            } else if (mission.currentProgress >= mission.targetAmount) {
                completeMission(mission);
            }
        }
    }

    // Remove completed missions
    activeMissions.erase(
        std::remove_if(activeMissions.begin(), activeMissions.end(),
            [](const Mission& m) { return m.completed; }),
        activeMissions.end()
    );
}

void AdvancedFactionManager::completeMission(Mission& mission) {
    mission.completed = true;

    // Grant rewards
    for (const auto& pair : mission.resourceRewards) {
        int index = static_cast<int>(pair.first);
        if (index < resources.size()) {
            resources[index] += pair.second;
        }
    }

    std::cout << "Mission completed: " << mission.description << "!" << std::endl;
}

void AdvancedFactionManager::updatePopulation(float deltaTime) {
    populationStats.population = static_cast<int>(agents.size());

    // Natural population growth/decline (handled by agent spawning)
    // But we update the statistics
    float growthRate = populationStats.getGrowthRate();

    // Update happiness factors
    calculateHappiness();

    // Literacy and health improve with funding
    populationStats.literacy += educationFunding * deltaTime * 0.1f;
    populationStats.health += healthcareFunding * deltaTime * 0.1f;

    populationStats.literacy = std::min(100.0f, populationStats.literacy);
    populationStats.health = std::min(100.0f, populationStats.health);
}

void AdvancedFactionManager::calculateHappiness() {
    float happiness = 50.0f; // Base happiness

    // Positive factors
    happiness += populationStats.health * 0.2f;
    happiness += populationStats.literacy * 0.15f;

    // Resource availability
    float totalResources = getTotalResources();
    float avgResourcesPerPop = populationStats.population > 0 ?
                               totalResources / populationStats.population : 0;
    happiness += std::min(20.0f, avgResourcesPerPop * 0.1f);

    // Negative factors - war
    int warsCount = 0;
    for (const auto& pair : diplomaticRelations) {
        if (pair.second.relation == DiplomaticRelation::WAR) {
            warsCount++;
        }
    }
    happiness -= warsCount * 10.0f;

    populationStats.happiness = std::min(100.0f, std::max(0.0f, happiness));
}

void AdvancedFactionManager::advancedUpdate(float deltaTime) {
    update(deltaTime);
    updateDiplomacy(deltaTime);
    updateResearch(deltaTime);
    updateTrades(deltaTime);
    updateCombatUnits(deltaTime);
    updateMissions(deltaTime);
    updatePopulation(deltaTime);
}

// AdvancedGameManager implementation

AdvancedGameManager::AdvancedGameManager()
    : weatherChangeTimer(0), currentSeason(Season::SPRING), seasonTimer(0) {
    currentWeather = createWeatherEffect(WeatherType::CLEAR);
}

void AdvancedGameManager::updateWeather(float deltaTime) {
    currentWeather.timeRemaining -= deltaTime;

    if (currentWeather.timeRemaining <= 0) {
        // Random weather change
        int weatherChoice = rand() % 6;
        changeWeather(static_cast<WeatherType>(weatherChoice));
    }

    weatherChangeTimer += deltaTime;
}

void AdvancedGameManager::changeWeather(WeatherType newWeather) {
    currentWeather = createWeatherEffect(newWeather);
    std::cout << "Weather changed to: " << static_cast<int>(newWeather) << std::endl;
}

void AdvancedGameManager::updateSeason(float deltaTime) {
    seasonTimer += deltaTime;

    // Each season lasts 180 seconds (3 minutes)
    if (seasonTimer >= 180.0f) {
        seasonTimer = 0;
        int nextSeason = (static_cast<int>(currentSeason) + 1) % 4;
        currentSeason = static_cast<Season>(nextSeason);
        std::cout << "Season changed to: " << static_cast<int>(currentSeason) << std::endl;
    }
}

float AdvancedGameManager::getSeasonModifier(const std::string& category) const {
    if (category == "farming") {
        switch (currentSeason) {
            case Season::SPRING: return 1.3f;
            case Season::SUMMER: return 1.5f;
            case Season::FALL: return 1.2f;
            case Season::WINTER: return 0.3f;
        }
    } else if (category == "movement") {
        switch (currentSeason) {
            case Season::WINTER: return 0.7f;
            default: return 1.0f;
        }
    }

    return 1.0f;
}

void AdvancedGameManager::triggerEvent(const GlobalEvent& event) {
    activeEvents.push_back(event);
    std::cout << "Global event triggered: " << event.name << std::endl;
    std::cout << "  " << event.description << std::endl;
}

void AdvancedGameManager::updateEvents(float deltaTime, World* world) {
    for (auto it = activeEvents.begin(); it != activeEvents.end();) {
        it->duration -= deltaTime;

        if (it->effect) {
            it->effect(world);
        }

        if (it->duration <= 0) {
            std::cout << "Event ended: " << it->name << std::endl;
            it = activeEvents.erase(it);
        } else {
            ++it;
        }
    }
}

bool AdvancedGameManager::checkVictoryConditions(const World* world, Faction& winner) {
    // Domination victory - eliminate all other factions
    int activeFactions = 0;
    Faction lastFaction = Faction::RED;

    for (const auto& factionMgr : world->factions) {
        if (factionMgr->getPopulation() > 0) {
            activeFactions++;
            lastFaction = factionMgr->faction;
        }
    }

    if (activeFactions == 1) {
        winner = lastFaction;
        return true;
    }

    // TODO: Add more victory conditions (science, cultural, economic, etc.)

    return false;
}

void AdvancedGameManager::update(float deltaTime, World* world) {
    updateWeather(deltaTime);
    updateSeason(deltaTime);
    updateEvents(deltaTime, world);
}

// MarketSystem implementation

MarketSystem::MarketSystem() {
    // Initialize prices
    prices[TradeResourceType::FOOD] = 1.0f;
    prices[TradeResourceType::WOOD] = 1.5f;
    prices[TradeResourceType::STONE] = 2.0f;
    prices[TradeResourceType::GOLD] = 10.0f;
    prices[TradeResourceType::WEAPONS] = 5.0f;
    prices[TradeResourceType::LUXURY_GOODS] = 8.0f;

    // Initialize supply and demand
    for (auto& pair : prices) {
        globalSupply[pair.first] = 1000;
        globalDemand[pair.first] = 1000;
    }
}

void MarketSystem::updatePrices(float deltaTime) {
    for (auto& pair : prices) {
        TradeResourceType resource = pair.first;
        int supply = globalSupply[resource];
        int demand = globalDemand[resource];

        if (supply == 0) supply = 1; // Avoid division by zero

        // Price = basePrice * (demand / supply)
        float supplyDemandRatio = static_cast<float>(demand) / supply;
        float basePrice = 1.0f;

        switch (resource) {
            case TradeResourceType::GOLD: basePrice = 10.0f; break;
            case TradeResourceType::WEAPONS: basePrice = 5.0f; break;
            case TradeResourceType::LUXURY_GOODS: basePrice = 8.0f; break;
            case TradeResourceType::STONE: basePrice = 2.0f; break;
            case TradeResourceType::WOOD: basePrice = 1.5f; break;
            default: basePrice = 1.0f; break;
        }

        float targetPrice = basePrice * supplyDemandRatio;

        // Smoothly adjust price toward target
        float priceDiff = targetPrice - pair.second;
        pair.second += priceDiff * deltaTime * 0.1f;

        // Clamp price to reasonable range
        pair.second = std::max(0.1f, std::min(100.0f, pair.second));
    }

    // Decay supply and demand back toward equilibrium
    for (auto& pair : globalSupply) {
        int diff = 1000 - pair.second;
        pair.second += static_cast<int>(diff * deltaTime * 0.05f);
    }

    for (auto& pair : globalDemand) {
        int diff = 1000 - pair.second;
        pair.second += static_cast<int>(diff * deltaTime * 0.05f);
    }
}

float MarketSystem::getPrice(TradeResourceType resource) const {
    auto it = prices.find(resource);
    return it != prices.end() ? it->second : 1.0f;
}

void MarketSystem::addSupply(TradeResourceType resource, int amount) {
    globalSupply[resource] += amount;
}

void MarketSystem::addDemand(TradeResourceType resource, int amount) {
    globalDemand[resource] += amount;
}

// Technology initialization

void initializeTechnologies(std::map<TechnologyType, Technology>& techTree) {
    // Military technologies
    Technology bronzeWeapons;
    bronzeWeapons.type = TechnologyType::BRONZE_WEAPONS;
    bronzeWeapons.name = "Bronze Weapons";
    bronzeWeapons.description = "Basic metalworking for weapons";
    bronzeWeapons.researchCost = 100;
    bronzeWeapons.militaryBonus = 0.15f;
    techTree[TechnologyType::BRONZE_WEAPONS] = bronzeWeapons;

    Technology ironWeapons;
    ironWeapons.type = TechnologyType::IRON_WEAPONS;
    ironWeapons.name = "Iron Weapons";
    ironWeapons.description = "Advanced metalworking for stronger weapons";
    ironWeapons.researchCost = 250;
    ironWeapons.prerequisites.push_back(TechnologyType::BRONZE_WEAPONS);
    ironWeapons.militaryBonus = 0.30f;
    techTree[TechnologyType::IRON_WEAPONS] = ironWeapons;

    // Economic technologies
    Technology agriculture;
    agriculture.type = TechnologyType::AGRICULTURE;
    agriculture.name = "Agriculture";
    agriculture.description = "Organized farming techniques";
    agriculture.researchCost = 80;
    agriculture.economicBonus = 0.20f;
    techTree[TechnologyType::AGRICULTURE] = agriculture;

    Technology mining;
    mining.type = TechnologyType::MINING;
    mining.name = "Mining";
    mining.description = "Extract resources from the earth";
    mining.researchCost = 120;
    mining.economicBonus = 0.25f;
    techTree[TechnologyType::MINING] = mining;

    // Civic technologies
    Technology writing;
    writing.type = TechnologyType::WRITING;
    writing.name = "Writing";
    writing.description = "Record knowledge and history";
    writing.researchCost = 150;
    techTree[TechnologyType::WRITING] = writing;

    Technology medicine;
    medicine.type = TechnologyType::MEDICINE;
    medicine.name = "Medicine";
    medicine.description = "Improve population health";
    medicine.researchCost = 200;
    medicine.healthBonus = 0.30f;
    techTree[TechnologyType::MEDICINE] = medicine;

    // Add more technologies...
}

// Weather effect presets

WeatherEffect createWeatherEffect(WeatherType type) {
    WeatherEffect effect;
    effect.type = type;

    switch (type) {
        case WeatherType::CLEAR:
            effect.intensity = 0.0f;
            effect.duration = 120.0f;
            effect.timeRemaining = 120.0f;
            break;

        case WeatherType::RAIN:
            effect.intensity = 0.5f;
            effect.duration = 60.0f;
            effect.timeRemaining = 60.0f;
            effect.movementSpeedModifier = 0.9f;
            effect.farmProductivityModifier = 1.2f;
            effect.visibilityModifier = 0.8f;
            break;

        case WeatherType::STORM:
            effect.intensity = 0.9f;
            effect.duration = 30.0f;
            effect.timeRemaining = 30.0f;
            effect.movementSpeedModifier = 0.6f;
            effect.farmProductivityModifier = 0.7f;
            effect.visibilityModifier = 0.5f;
            effect.combatEffectivenessModifier = 0.7f;
            break;

        case WeatherType::SNOW:
            effect.intensity = 0.6f;
            effect.duration = 90.0f;
            effect.timeRemaining = 90.0f;
            effect.movementSpeedModifier = 0.7f;
            effect.farmProductivityModifier = 0.3f;
            effect.visibilityModifier = 0.7f;
            break;

        case WeatherType::FOG:
            effect.intensity = 0.4f;
            effect.duration = 45.0f;
            effect.timeRemaining = 45.0f;
            effect.visibilityModifier = 0.4f;
            effect.combatEffectivenessModifier = 0.8f;
            break;

        case WeatherType::DROUGHT:
            effect.intensity = 0.8f;
            effect.duration = 180.0f;
            effect.timeRemaining = 180.0f;
            effect.farmProductivityModifier = 0.4f;
            break;
    }

    return effect;
}
