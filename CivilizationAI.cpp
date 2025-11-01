#include "CivilizationAI.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>

// Static member definitions
int World::WORLD_SIZE = 500;
int World::MAX_AGENTS_PER_FACTION = 100;

// Neural Layer Implementation
NeuralLayer::NeuralLayer(int inputSize, int outputSize) {
    weights.resize(outputSize, std::vector<float>(inputSize));
    biases.resize(outputSize);
    outputs.resize(outputSize);

    // Xavier initialization
    std::random_device rd;
    std::mt19937 gen(rd());
    float range = sqrt(6.0f / (inputSize + outputSize));
    std::uniform_real_distribution<float> dist(-range, range);

    for (auto& row : weights) {
        for (auto& w : row) {
            w = dist(gen);
        }
    }
    for (auto& b : biases) {
        b = dist(gen) * 0.1f;
    }
}

std::vector<float> NeuralLayer::forward(const std::vector<float>& inputs) {
    for (size_t i = 0; i < outputs.size(); ++i) {
        float sum = biases[i];
        for (size_t j = 0; j < inputs.size(); ++j) {
            sum += weights[i][j] * inputs[j];
        }
        outputs[i] = activation(sum);
    }
    return outputs;
}

void NeuralLayer::mutate(float mutationRate, float mutationStrength) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> probDist(0, 1);
    std::normal_distribution<float> mutDist(0, mutationStrength);

    for (auto& row : weights) {
        for (auto& w : row) {
            if (probDist(gen) < mutationRate) {
                w += mutDist(gen);
                w = std::max(-5.0f, std::min(5.0f, w)); // Clamp weights
            }
        }
    }
    for (auto& b : biases) {
        if (probDist(gen) < mutationRate) {
            b += mutDist(gen);
            b = std::max(-5.0f, std::min(5.0f, b));
        }
    }
}

// Neural Network Implementation
NeuralNetwork::NeuralNetwork(std::vector<int> topology) : fitness(0) {
    for (size_t i = 0; i < topology.size() - 1; ++i) {
        layers.emplace_back(topology[i], topology[i + 1]);
    }
}

std::vector<float> NeuralNetwork::predict(const std::vector<float>& inputs) {
    std::vector<float> current = inputs;
    for (auto& layer : layers) {
        current = layer.forward(current);
    }
    return current;
}

void NeuralNetwork::mutate(float mutationRate, float mutationStrength) {
    for (auto& layer : layers) {
        layer.mutate(mutationRate, mutationStrength);
    }
}

NeuralNetwork NeuralNetwork::crossover(const NeuralNetwork& other) const {
    NeuralNetwork child = *this;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0, 1);

    for (size_t l = 0; l < layers.size(); ++l) {
        for (size_t i = 0; i < layers[l].weights.size(); ++i) {
            if (dist(gen) < 0.5f) {
                child.layers[l].weights[i] = other.layers[l].weights[i];
                child.layers[l].biases[i] = other.layers[l].biases[i];
            }
        }
    }
    return child;
}

// Building Implementation
Building::Building(Type t, Vector3 pos, Faction f)
    : type(t), position(pos), faction(f), occupants(0), health(100.0f) {
    switch (type) {
    case HOUSE: capacity = 4; break;
    case FARM: capacity = 3; break;
    case MINE: capacity = 5; break;
    case BARRACKS: capacity = 10; break;
    case GOVERNMENT_CENTER: capacity = 8; break;
    case STORAGE: capacity = 2; break;
    }
}

void Building::update(float deltaTime) {
    // Slowly repair if not at full health
    if (health < 100.0f && health > 0) {
        health += deltaTime * 0.5f; // Slow repair
        health = std::min(100.0f, health);
    }
}

// Agent Implementation
Agent::Agent(Vector3 pos, Faction f, std::unique_ptr<NeuralNetwork> nn)
    : position(pos), faction(f), role(Role::CITIZEN),
    health(100.0f), energy(100.0f), age(0),
    home(nullptr), workplace(nullptr), brain(std::move(nn)),
    targetEnemy(nullptr), groundHeight(0.0f), isGrounded(true),
    verticalVelocity(0.0f), inCombat(false), combatCooldown(0.0f),
    kills(0), reproductionTimer(0.0f) {

    carriedResources.fill(0);
    memory.resize(10, 0.0f); // Simple memory vector

    // Keep position on ground
    position.z = groundHeight;
}

void Agent::update(float deltaTime, const std::vector<Agent*>& nearbyAgents,
    const std::vector<Building*>& nearbyBuildings) {
    // Age and energy management
    age++;
    energy -= deltaTime * 0.1f;
    if (energy < 0) {
        health -= deltaTime * 2.0f;
    }

    // Update combat cooldown
    if (combatCooldown > 0) {
        combatCooldown -= deltaTime;
    }
    inCombat = false; // Reset combat state

    // Update reproduction timer
    reproductionTimer += deltaTime;

    // Make AI decision
    makeDecision(nearbyAgents, nearbyBuildings);

    // Execute movement
    move(deltaTime);

    // Role-specific behaviors
    switch (role) {
    case Role::WORKER:
        if (workplace) {
            // Work logic
            if ((position - workplace->position).length() < 2.0f) {
                collectResource(ResourceType::FOOD); // Simplified
            }
        }
        break;

    case Role::SOLDIER:
        if (targetEnemy && targetEnemy->isAlive()) {
            if ((position - targetEnemy->position).length() < 1.5f) {
                attack(targetEnemy);
            }
        }
        break;

    case Role::GOVERNMENT:
        // Government officials work from government buildings
        if (workplace && workplace->type == Building::GOVERNMENT_CENTER) {
            energy += deltaTime * 0.2f; // Less energy drain
        }
        break;

    default:
        break;
    }
}

void Agent::makeDecision(const std::vector<Agent*>& nearbyAgents,
    const std::vector<Building*>& nearbyBuildings) {
    // Get state vector and run through neural network
    std::vector<float> state = getStateVector(nearbyAgents, nearbyBuildings);
    std::vector<float> outputs = brain->predict(state);

    // Interpret neural network outputs
    // Output 0-2: Movement direction (x, y, z)
    // Output 3: Role change desire
    // Output 4: Attack aggression
    // Output 5: Resource collection priority
    // Output 6-8: Target selection weights

    if (outputs.size() >= 9) {
        // Movement decision
        Vector3 desiredVelocity(outputs[0], outputs[1], outputs[2] * 0.3f);
        velocity = desiredVelocity.normalize() * 10.0f;

        // Role change consideration
        if (outputs[3] > 0.5f && role == Role::CITIZEN) {
            float randValue = static_cast<float>(std::rand()) / RAND_MAX;
            if (randValue < 0.3f) changeRole(Role::WORKER);
            else if (randValue < 0.5f) changeRole(Role::SOLDIER);
        }

        // Combat decision
        if (role == Role::SOLDIER && outputs[4] > 0.3f) {
            // Find nearest enemy
            targetEnemy = nullptr;
            float minDist = 50.0f;
            for (Agent* other : nearbyAgents) {
                if (other->faction != faction && other->isAlive()) {
                    float dist = (position - other->position).length();
                    if (dist < minDist) {
                        minDist = dist;
                        targetEnemy = other;
                    }
                }
            }
        }

        // Update memory
        for (size_t i = 0; i < memory.size() - 1; ++i) {
            memory[i] = memory[i + 1];
        }
        memory.back() = outputs[5];
    }
}

void Agent::move(float deltaTime) {
    // Apply velocity
    position = position + velocity * deltaTime;

    // Apply gravity if not grounded
    if (!isGrounded) {
        verticalVelocity -= 9.8f * deltaTime;
        position.z += verticalVelocity * deltaTime;
    }

    // Check ground collision
    if (position.z <= groundHeight) {
        position.z = groundHeight;
        verticalVelocity = 0;
        isGrounded = true;
    }

    // Keep within expanded world bounds
    position.x = std::max(0.0f, std::min((float)World::WORLD_SIZE, position.x));
    position.y = std::max(0.0f, std::min((float)World::WORLD_SIZE, position.y));
    position.z = std::max(0.0f, std::min(20.0f, position.z));

    // Friction
    velocity = velocity * 0.95f;

    // Always keep agents grounded (no flying)
    position.z = 0.0f;
}

void Agent::changeRole(Role newRole) {
    role = newRole;

    // Role change effects
    switch (newRole) {
    case Role::SOLDIER:
        health = std::min(150.0f, health * 1.2f); // Soldiers are tougher
        break;
    case Role::WORKER:
        energy = std::min(120.0f, energy * 1.1f); // Workers have more stamina
        break;
    default:
        break;
    }
}

void Agent::attack(Agent* enemy) {
    if (!enemy || !enemy->isAlive()) return;
    if (combatCooldown > 0) return; // Still on cooldown

    float damage = 10.0f;
    if (role == Role::SOLDIER) damage *= 2.0f;

    enemy->health -= damage;
    energy -= 5.0f;

    // Set combat state
    inCombat = true;
    combatCooldown = 1.0f; // 1 second cooldown

    // Memory update - remember combat
    memory[0] = 1.0f;

    // Check for kill
    if (enemy->health <= 0) {
        kills++;
        energy += 10.0f; // Energy bonus for kill
    }
}

void Agent::collectResource(ResourceType type) {
    int index = static_cast<int>(type);
    if (carriedResources[index] < 10) {
        carriedResources[index]++;
        energy -= 2.0f;
    }
}

void Agent::depositResource(Building* storage) {
    if (!storage || storage->type != Building::STORAGE) return;

    // Transfer all carried resources
    for (auto& resource : carriedResources) {
        resource = 0;
    }
    energy += 5.0f; // Reward for depositing
}

std::string Agent::getResourceText() const {
    std::stringstream ss;

    if (carriedResources[0] > 0) ss << "F:" << carriedResources[0] << " ";
    if (carriedResources[1] > 0) ss << "W:" << carriedResources[1] << " ";
    if (carriedResources[2] > 0) ss << "S:" << carriedResources[2] << " ";
    if (carriedResources[3] > 0) ss << "G:" << carriedResources[3] << " ";
    if (carriedResources[4] > 0) ss << "A:" << carriedResources[4] << " ";

    return ss.str();
}

std::vector<float> Agent::getStateVector(const std::vector<Agent*>& nearbyAgents,
    const std::vector<Building*>& nearbyBuildings) const {
    std::vector<float> state;

    // Self state (10 values)
    state.push_back(position.x / 100.0f);
    state.push_back(position.y / 100.0f);
    state.push_back(position.z / 20.0f);
    state.push_back(health / 100.0f);
    state.push_back(energy / 100.0f);
    state.push_back(static_cast<float>(role) / 4.0f);

    // Resources carried (5 values)
    for (int r : carriedResources) {
        state.push_back(r / 10.0f);
    }

    // Nearby agents summary (12 values)
    int allyCount = 0, enemyCount = 0;
    float nearestAllyDist = 100.0f, nearestEnemyDist = 100.0f;
    Vector3 avgAllyPos, avgEnemyPos;

    for (Agent* other : nearbyAgents) {
        float dist = (position - other->position).length();
        if (other->faction == faction) {
            allyCount++;
            avgAllyPos = avgAllyPos + other->position;
            nearestAllyDist = std::min(nearestAllyDist, dist);
        }
        else {
            enemyCount++;
            avgEnemyPos = avgEnemyPos + other->position;
            nearestEnemyDist = std::min(nearestEnemyDist, dist);
        }
    }

    state.push_back(allyCount / 10.0f);
    state.push_back(enemyCount / 10.0f);
    state.push_back(nearestAllyDist / 50.0f);
    state.push_back(nearestEnemyDist / 50.0f);

    if (allyCount > 0) {
        avgAllyPos = avgAllyPos * (1.0f / allyCount);
        state.push_back(avgAllyPos.x / 100.0f);
        state.push_back(avgAllyPos.y / 100.0f);
        state.push_back(avgAllyPos.z / 20.0f);
    }
    else {
        state.push_back(0);
        state.push_back(0);
        state.push_back(0);
    }

    if (enemyCount > 0) {
        avgEnemyPos = avgEnemyPos * (1.0f / enemyCount);
        state.push_back(avgEnemyPos.x / 100.0f);
        state.push_back(avgEnemyPos.y / 100.0f);
        state.push_back(avgEnemyPos.z / 20.0f);
    }
    else {
        state.push_back(0);
        state.push_back(0);
        state.push_back(0);
    }

    // Nearby buildings (8 values)
    int friendlyBuildings = 0, enemyBuildings = 0;
    float nearestBuildingDist = 100.0f;
    Building* nearestBuilding = nullptr;

    for (Building* building : nearbyBuildings) {
        float dist = (position - building->position).length();
        if (building->faction == faction) {
            friendlyBuildings++;
        }
        else {
            enemyBuildings++;
        }
        if (dist < nearestBuildingDist) {
            nearestBuildingDist = dist;
            nearestBuilding = building;
        }
    }

    state.push_back(friendlyBuildings / 5.0f);
    state.push_back(enemyBuildings / 5.0f);
    state.push_back(nearestBuildingDist / 50.0f);

    if (nearestBuilding) {
        state.push_back(static_cast<float>(nearestBuilding->type) / 6.0f);
        state.push_back(nearestBuilding->position.x / 100.0f);
        state.push_back(nearestBuilding->position.y / 100.0f);
        state.push_back(nearestBuilding->health / 100.0f);
        state.push_back(nearestBuilding->faction == faction ? 1.0f : -1.0f);
    }
    else {
        for (int i = 0; i < 5; ++i) state.push_back(0);
    }

    // Memory (10 values)
    for (float mem : memory) {
        state.push_back(mem);
    }

    return state; // Total: 48 inputs
}

// Faction Manager Implementation
FactionManager::FactionManager(Faction f) : faction(f), taxRate(0.1f),
militaryFunding(0.3f), constructionPriority(0.5f) {

    resources.fill(100); // Start with some resources

    // Create government neural network
    governmentBrain = std::make_unique<NeuralNetwork>(
        std::vector<int>{20, 16, 8, 4} // Government decision network
    );
}

void FactionManager::update(float deltaTime) {
    makeGovernmentDecisions();
    assignRoles();

    // Collect taxes and distribute resources
    for (Agent* agent : agents) {
        if (agent->isAlive() && agent->role == Role::WORKER) {
            for (size_t i = 0; i < resources.size(); ++i) {
                int collected = agent->carriedResources[i];
                int taxed = static_cast<int>(collected * taxRate);
                resources[i] += taxed;
                agent->carriedResources[i] -= taxed;
            }
        }
    }
}

void FactionManager::makeGovernmentDecisions() {
    // Prepare state vector for government brain
    std::vector<float> state;

    // Population stats
    state.push_back(agents.size() / 100.0f);
    state.push_back(getSoldierCount() / 50.0f);
    state.push_back(getWorkerCount() / 50.0f);
    state.push_back(getAverageHealth() / 100.0f);

    // Resources
    for (int r : resources) {
        state.push_back(r / 1000.0f);
    }

    // Buildings
    state.push_back(buildings.size() / 20.0f);

    // Threat level (simplified)
    float threatLevel = 0.5f; // Would calculate based on enemy proximity
    state.push_back(threatLevel);

    // Pad to expected input size
    while (state.size() < 20) {
        state.push_back(0.0f);
    }

    // Get decisions from neural network
    std::vector<float> decisions = governmentBrain->predict(state);

    if (decisions.size() >= 3) {
        taxRate = std::max(0.0f, std::min(0.5f, decisions[0]));
        militaryFunding = std::max(0.0f, std::min(1.0f, decisions[1]));
        constructionPriority = std::max(0.0f, std::min(1.0f, decisions[2]));
    }
}

void FactionManager::assignRoles() {
    int targetSoldiers = static_cast<int>(agents.size() * militaryFunding);
    int currentSoldiers = getSoldierCount();

    if (currentSoldiers < targetSoldiers) {
        // Convert citizens to soldiers
        for (Agent* agent : agents) {
            if (agent->role == Role::CITIZEN && currentSoldiers < targetSoldiers) {
                agent->changeRole(Role::SOLDIER);
                currentSoldiers++;
            }
        }
    }
    else if (currentSoldiers > targetSoldiers) {
        // Convert soldiers to workers
        for (Agent* agent : agents) {
            if (agent->role == Role::SOLDIER && currentSoldiers > targetSoldiers) {
                agent->changeRole(Role::WORKER);
                currentSoldiers--;
            }
        }
    }
}

int FactionManager::getSoldierCount() const {
    return std::count_if(agents.begin(), agents.end(),
        [](Agent* a) { return a->role == Role::SOLDIER && a->isAlive(); });
}

int FactionManager::getWorkerCount() const {
    return std::count_if(agents.begin(), agents.end(),
        [](Agent* a) { return a->role == Role::WORKER && a->isAlive(); });
}

float FactionManager::getAverageHealth() const {
    if (agents.empty()) return 0;
    float total = 0;
    for (Agent* a : agents) {
        if (a->isAlive()) total += a->health;
    }
    return total / agents.size();
}

// World Implementation
World::World() : currentGeneration(0), generationTimer(0),
uniformDist(0, 1), timeOfDay(12.0f), weatherIntensity(0.5f) {
    std::random_device rd;
    rng.seed(rd());

    // Also seed the C random number generator
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void World::initialize() {
    // Initialize gene pool FIRST with random networks
    for (int i = 0; i < 100; ++i) {
        genePool.emplace_back(std::vector<int>{48, 32, 24, 16, 9});
    }

    // Create three factions
    factions.push_back(std::make_unique<FactionManager>(Faction::RED));
    factions.push_back(std::make_unique<FactionManager>(Faction::BLUE));
    factions.push_back(std::make_unique<FactionManager>(Faction::GREEN));

    // Create initial population for each faction
    for (size_t f = 0; f < factions.size(); ++f) {
        // Starting position for each faction (triangle formation in larger world)
        Vector3 basePos;
        switch (f) {
        case 0: basePos = Vector3(100, 100, 0); break;  // Red - bottom left
        case 1: basePos = Vector3(400, 100, 0); break;  // Blue - bottom right
        case 2: basePos = Vector3(250, 400, 0); break;  // Green - top center
        }

        // Create government center
        auto govCenter = std::make_unique<Building>(
            Building::GOVERNMENT_CENTER, basePos, static_cast<Faction>(f));
        factions[f]->buildings.push_back(govCenter.get());
        allBuildings.push_back(std::move(govCenter));

        // Create houses around the center
        for (int i = 0; i < 5; ++i) {
            float angle = i * 2 * 3.14159f / 5;
            Vector3 housePos = basePos + Vector3(cos(angle) * 10, sin(angle) * 10, 0);

            auto house = std::make_unique<Building>(
                Building::HOUSE, housePos, static_cast<Faction>(f));
            factions[f]->buildings.push_back(house.get());
            allBuildings.push_back(std::move(house));
        }

        // Create initial agents
        for (int i = 0; i < 15; ++i) {
            Vector3 agentPos = basePos + Vector3(
                uniformDist(rng) * 20 - 10,
                uniformDist(rng) * 20 - 10,
                0
            );

            spawnAgent(static_cast<Faction>(f), agentPos);
        }
    }
}

void World::update(float deltaTime) {
    // Update time of day
    timeOfDay += deltaTime * 0.1f; // 10x speed
    if (timeOfDay >= 24.0f) timeOfDay -= 24.0f;

    // Update generation timer
    generationTimer += deltaTime;
    if (generationTimer >= GENERATION_TIME) {
        evolveGeneration();
        generationTimer = 0;
        currentGeneration++;
    }

    // Update all agents
    for (auto& agent : allAgents) {
        if (agent->isAlive()) {
            auto nearbyAgents = getAgentsNear(agent->position, 20.0f);
            auto nearbyBuildings = getBuildingsNear(agent->position, 30.0f);
            agent->update(deltaTime, nearbyAgents, nearbyBuildings);
        }
    }

    // Update buildings
    for (auto& building : allBuildings) {
        building->update(deltaTime);
    }

    // Update faction managers
    for (auto& faction : factions) {
        faction->update(deltaTime);
    }

    // Handle combat
    handleCombat();

    // Clean up dead agents
    allAgents.erase(
        std::remove_if(allAgents.begin(), allAgents.end(),
            [](const std::unique_ptr<Agent>& a) { return !a->isAlive(); }),
        allAgents.end()
    );

    // Update faction agent lists
    for (auto& faction : factions) {
        faction->agents.erase(
            std::remove_if(faction->agents.begin(), faction->agents.end(),
                [](Agent* a) { return !a->isAlive(); }),
            faction->agents.end()
        );
    }
}

void World::evolveGeneration() {
    // Evaluate fitness for all agents
    std::vector<std::pair<float, NeuralNetwork*>> fitnessScores;

    for (auto& agent : allAgents) {
        if (agent->isAlive()) {
            float fitness = agent->health + agent->energy;
            fitness += agent->age * 0.01f;

            // Role bonuses
            if (agent->role == Role::SOLDIER) fitness *= 1.2f;
            if (agent->role == Role::GOVERNMENT) fitness *= 1.5f;

            // Resource bonus
            for (int r : agent->carriedResources) {
                fitness += r * 10;
            }

            agent->brain->fitness = fitness;
            fitnessScores.push_back({ fitness, agent->brain.get() });
        }
    }

    // Sort by fitness
    std::sort(fitnessScores.begin(), fitnessScores.end(),
        [](const auto& a, const auto& b) { return a.first > b.first; });

    // Create new gene pool through selection and crossover
    std::vector<NeuralNetwork> newGenePool;

    // Keep top 20% elite
    size_t eliteCount = fitnessScores.size() / 5;
    for (size_t i = 0; i < eliteCount && i < fitnessScores.size(); ++i) {
        newGenePool.push_back(*fitnessScores[i].second);
    }

    // Fill rest with offspring
    while (newGenePool.size() < 100) {
        // Tournament selection
        NeuralNetwork* parent1 = nullptr;
        NeuralNetwork* parent2 = nullptr;

        if (fitnessScores.size() >= 2) {
            int idx1 = std::rand() % std::min(size_t(10), fitnessScores.size());
            int idx2 = std::rand() % std::min(size_t(10), fitnessScores.size());
            parent1 = fitnessScores[idx1].second;
            parent2 = fitnessScores[idx2].second;

            NeuralNetwork child = parent1->crossover(*parent2);
            child.mutate(0.1f, 0.2f);
            newGenePool.push_back(child);
        }
        else {
            // Random network if not enough parents
            newGenePool.emplace_back(std::vector<int>{48, 32, 24, 16, 9});
        }
    }

    genePool = newGenePool;

    // Respawn some agents with new brains
    for (auto& faction : factions) {
        int toSpawn = MAX_AGENTS_PER_FACTION - faction->agents.size();
        for (int i = 0; i < toSpawn / 2; ++i) {  // Spawn half the deficit
            if (!faction->buildings.empty()) {
                Vector3 spawnPos = faction->buildings[0]->position;
                spawnPos.x += uniformDist(rng) * 10 - 5;
                spawnPos.y += uniformDist(rng) * 10 - 5;
                spawnAgent(faction->faction, spawnPos);
            }
        }
    }
}

void World::spawnAgent(Faction f, Vector3 position) {
    // Safety check: ensure gene pool is not empty
    if (genePool.empty()) {
        // Create a default neural network if gene pool is empty
        genePool.emplace_back(std::vector<int>{48, 32, 24, 16, 9});
    }

    // Select brain from gene pool
    int brainIdx = std::rand() % genePool.size();
    auto brain = std::make_unique<NeuralNetwork>(genePool[brainIdx]);

    auto agent = std::make_unique<Agent>(position, f, std::move(brain));

    // Assign to faction
    factions[static_cast<int>(f)]->agents.push_back(agent.get());

    // Find a home
    for (auto& building : allBuildings) {
        if (building->faction == f && building->type == Building::HOUSE
            && building->canAcceptOccupant()) {
            agent->home = building.get();
            building->occupants++;
            break;
        }
    }

    allAgents.push_back(std::move(agent));
}

void World::handleCombat() {
    // Simple combat resolution
    for (auto& agent : allAgents) {
        if (agent->role == Role::SOLDIER && agent->targetEnemy) {
            if (!agent->targetEnemy->isAlive()) {
                agent->targetEnemy = nullptr;
            }
        }
    }
}

std::vector<Agent*> World::getAgentsNear(Vector3 position, float radius) const {
    std::vector<Agent*> result;
    float radiusSq = radius * radius;

    for (auto& agent : allAgents) {
        if (agent->isAlive()) {
            Vector3 diff = agent->position - position;
            float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
            if (distSq <= radiusSq) {
                result.push_back(agent.get());
            }
        }
    }

    return result;
}

std::vector<Building*> World::getBuildingsNear(Vector3 position, float radius) const {
    std::vector<Building*> result;
    float radiusSq = radius * radius;

    for (auto& building : allBuildings) {
        Vector3 diff = building->position - position;
        float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
        if (distSq <= radiusSq) {
            result.push_back(building.get());
        }
    }

    return result;
}

float World::evaluateFactionFitness(Faction f) const {
    auto& faction = factions[static_cast<int>(f)];

    float fitness = 0;
    fitness += faction->agents.size() * 100;
    fitness += faction->buildings.size() * 200;
    fitness += faction->getAverageHealth();

    for (int r : faction->resources) {
        fitness += r;
    }

    return fitness;
}