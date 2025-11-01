#ifndef CIVILIZATION_AI_H
#define CIVILIZATION_AI_H

#include <vector>
#include <string>
#include <memory>
#include <random>
#include <cmath>
#include <array>
#include <cstdlib>

// Mathematical utilities
struct Vector3 {
    float x, y, z;
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }

    float length() const { return sqrt(x * x + y * y + z * z); }
    Vector3 normalize() const {
        float len = length();
        return len > 0 ? Vector3(x / len, y / len, z / len) : Vector3(0, 0, 0);
    }
};

// Resource types
enum class ResourceType {
    FOOD,
    WOOD,
    STONE,
    GOLD,
    WEAPONS,
    COUNT
};

// Agent roles
enum class Role {
    CITIZEN,
    WORKER,
    SOLDIER,
    GOVERNMENT,
    COUNT
};

// Faction identifiers
enum class Faction {
    RED,
    BLUE,
    GREEN,
    COUNT
};

// Neural Network Layer
class NeuralLayer {
public:
    std::vector<std::vector<float>> weights;
    std::vector<float> biases;
    std::vector<float> outputs;

    NeuralLayer(int inputSize, int outputSize);
    std::vector<float> forward(const std::vector<float>& inputs);
    void mutate(float mutationRate, float mutationStrength);

private:
    float activation(float x) { return tanh(x); }
};

// Neural Network for AI decision making
class NeuralNetwork {
public:
    std::vector<NeuralLayer> layers;
    float fitness;

    NeuralNetwork(std::vector<int> topology);
    std::vector<float> predict(const std::vector<float>& inputs);
    void mutate(float mutationRate = 0.1f, float mutationStrength = 0.3f);
    NeuralNetwork crossover(const NeuralNetwork& other) const;

    // Serialization for saving/loading
    std::vector<float> serialize() const;
    void deserialize(const std::vector<float>& data);
};

// Building types
class Building {
public:
    enum Type {
        HOUSE,
        FARM,
        MINE,
        BARRACKS,
        GOVERNMENT_CENTER,
        STORAGE
    };

    Type type;
    Vector3 position;
    Faction faction;
    int capacity;
    int occupants;
    float health;

    Building(Type t, Vector3 pos, Faction f);
    void update(float deltaTime);
    bool canAcceptOccupant() const { return occupants < capacity; }
};

// Individual agent with neural network brain
class Agent {
public:
    // Basic properties
    Vector3 position;
    Vector3 velocity;
    Faction faction;
    Role role;
    float health;
    float energy;
    int age;

    // Resources carried
    std::array<int, static_cast<int>(ResourceType::COUNT)> carriedResources;

    // Home and workplace
    Building* home;
    Building* workplace;

    // Neural network for decision making
    std::unique_ptr<NeuralNetwork> brain;

    // Memory and state
    std::vector<float> memory;

    // Physics
    Vector3 targetPosition;
    Agent* targetEnemy;
    float groundHeight;
    bool isGrounded;
    float verticalVelocity;

    // Enhanced state
    bool inCombat;
    float combatCooldown;
    int kills;
    float reproductionTimer;

    // Constructor
    Agent(Vector3 pos, Faction f, std::unique_ptr<NeuralNetwork> nn);

    // Core functions
    void update(float deltaTime, const std::vector<Agent*>& nearbyAgents,
        const std::vector<Building*>& nearbyBuildings);
    void makeDecision(const std::vector<Agent*>& nearbyAgents,
        const std::vector<Building*>& nearbyBuildings);
    void move(float deltaTime);
    void changeRole(Role newRole);
    void attack(Agent* enemy);
    void collectResource(ResourceType type);
    void depositResource(Building* storage);

    // State queries
    bool isAlive() const { return health > 0; }
    std::string getResourceText() const;

    // Neural network inputs
    std::vector<float> getStateVector(const std::vector<Agent*>& nearbyAgents,
        const std::vector<Building*>& nearbyBuildings) const;
};

// Faction management
class FactionManager {
public:
    Faction faction;
    std::vector<Agent*> agents;
    std::vector<Building*> buildings;
    std::array<int, static_cast<int>(ResourceType::COUNT)> resources;

    // Government policies (controlled by neural network)
    float taxRate;
    float militaryFunding;
    float constructionPriority;
    std::unique_ptr<NeuralNetwork> governmentBrain;

    FactionManager(Faction f);
    void update(float deltaTime);
    void makeGovernmentDecisions();
    void assignRoles();
    void constructBuilding(Building::Type type, Vector3 position);
    bool canAfford(Building::Type type) const;

    // Statistics
    int getPopulation() const { return agents.size(); }
    int getSoldierCount() const;
    int getWorkerCount() const;
    float getAverageHealth() const;
    float getTotalResources() const;
};

// Main world simulation
class World {
public:
    static int WORLD_SIZE;
    static int MAX_AGENTS_PER_FACTION;
    static constexpr int GENERATION_TIME = 300; // seconds per generation

    std::vector<std::unique_ptr<FactionManager>> factions;
    std::vector<std::unique_ptr<Agent>> allAgents;
    std::vector<std::unique_ptr<Building>> allBuildings;

    // Genetic algorithm parameters
    int currentGeneration;
    float generationTimer;
    std::vector<NeuralNetwork> genePool;

    // Random number generation
    std::mt19937 rng;
    std::uniform_real_distribution<float> uniformDist;

    // World state
    float timeOfDay; // 0-24 hours
    float weatherIntensity;

    World();
    void initialize();
    void update(float deltaTime);
    void evolveGeneration();
    void spawnAgent(Faction f, Vector3 position);
    void handleCombat();
    void distributeResources();

    // Queries
    std::vector<Agent*> getAgentsNear(Vector3 position, float radius) const;
    std::vector<Building*> getBuildingsNear(Vector3 position, float radius) const;

    // Statistics and fitness evaluation
    float evaluateFactionFitness(Faction f) const;
    void saveGeneration(const std::string& filename);
    void loadGeneration(const std::string& filename);
};

// Visualization data for rendering
struct VisualizationData {
    struct AgentVis {
        Vector3 position;
        Faction faction;
        Role role;
        float health;
        std::string resourceText;
        bool inCombat;
    };

    struct BuildingVis {
        Vector3 position;
        Building::Type type;
        Faction faction;
        float health;
        int occupancy;
    };

    std::vector<AgentVis> agents;
    std::vector<BuildingVis> buildings;
    int generation;
    std::array<int, 3> populations; // Per faction
    std::array<float, 3> avgFitness; // Per faction
    float timeOfDay;
};

#endif // CIVILIZATION_AI_H