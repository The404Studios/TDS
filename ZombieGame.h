#ifndef ZOMBIE_GAME_H
#define ZOMBIE_GAME_H

#include <vector>
#include <string>
#include <memory>
#include <cmath>
#include <array>

// Mathematical utilities
struct Vector3 {
    float x, y, z;
    Vector3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vector3 operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
    Vector3 operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
    Vector3 operator*(float s) const { return Vector3(x * s, y * s, z * s); }
    Vector3 operator/(float s) const { return s != 0 ? Vector3(x / s, y / s, z / s) : Vector3(); }

    float length() const { return sqrt(x * x + y * y + z * z); }
    Vector3 normalize() const {
        float len = length();
        return len > 0 ? Vector3(x / len, y / len, z / len) : Vector3(0, 0, 0);
    }

    float dot(const Vector3& v) const { return x * v.x + y * v.y + z * v.z; }
    Vector3 cross(const Vector3& v) const {
        return Vector3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
};

// Navigation mesh for zombie pathfinding
struct NavMeshNode {
    Vector3 position;
    std::vector<NavMeshNode*> neighbors;
    bool walkable;
    float cost;

    // A* pathfinding
    float gCost;  // Distance from start
    float hCost;  // Distance to end
    float fCost() const { return gCost + hCost; }
    NavMeshNode* parent;

    NavMeshNode() : walkable(true), cost(1.0f), gCost(0), hCost(0), parent(nullptr) {}
};

// Weapon types
enum class WeaponType {
    PISTOL,
    SHOTGUN,
    RIFLE,
    MACHINE_GUN,
    SNIPER,
    COUNT
};

// Weapon class
class Weapon {
public:
    WeaponType type;
    std::string name;
    int ammo;
    int maxAmmo;
    int clipSize;
    int currentClip;
    float damage;
    float fireRate;
    float reloadTime;
    float range;
    float accuracy;
    float lastFireTime;
    float currentReloadTime;
    bool reloading;

    Weapon(WeaponType t);
    bool canFire() const;
    void fire();
    void reload();
    void update(float deltaTime);
};

// Player class
class Player {
public:
    Vector3 position;
    Vector3 velocity;
    Vector3 forward;
    Vector3 right;
    Vector3 up;

    float yaw;    // Horizontal rotation
    float pitch;  // Vertical rotation

    float health;
    float maxHealth;
    float speed;
    float sprintMultiplier;
    bool sprinting;

    int money;
    int kills;

    std::vector<std::unique_ptr<Weapon>> weapons;
    int currentWeaponIndex;

    Player();
    void update(float deltaTime);
    void move(Vector3 direction, float deltaTime);
    void rotate(float deltaYaw, float deltaPitch);
    void updateVectors();
    Weapon* getCurrentWeapon();
    void switchWeapon(int index);
    void takeDamage(float amount);
};

// Zombie class with NavMesh pathfinding
class Zombie {
public:
    Vector3 position;
    Vector3 velocity;
    float health;
    float maxHealth;
    float speed;
    float damage;
    float attackRange;
    float attackCooldown;
    float lastAttackTime;
    bool isDead;

    // AI and pathfinding
    std::vector<NavMeshNode*> path;
    int currentPathNode;
    Vector3 targetPosition;

    // Animation
    float walkCycle;
    float deathTimer;

    Zombie(Vector3 startPos);
    void update(float deltaTime, Player* player, const std::vector<NavMeshNode*>& navMesh);
    void findPath(Vector3 start, Vector3 end, const std::vector<NavMeshNode*>& navMesh);
    void followPath(float deltaTime);
    void attack(Player* player, float deltaTime);
    void takeDamage(float amount);
};

// Base building types
enum class BuildingType {
    WALL,
    TURRET,
    AMMO_STATION,
    HEALTH_STATION,
    BARRICADE,
    COUNT
};

// Base building class
class BaseBuilding {
public:
    BuildingType type;
    Vector3 position;
    float health;
    float maxHealth;
    int cost;
    int level;
    bool active;

    // Turret specific
    float fireRange;
    float fireRate;
    float lastFireTime;
    float turretYaw;

    BaseBuilding(BuildingType t, Vector3 pos);
    void update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies);
    void upgrade();
    void takeDamage(float amount);
    const char* getName() const;
    int getUpgradeCost() const;
};

// Bullet/Projectile class
struct Bullet {
    Vector3 position;
    Vector3 velocity;
    float damage;
    float lifetime;
    bool fromPlayer;

    Bullet(Vector3 pos, Vector3 vel, float dmg, bool player = true)
        : position(pos), velocity(vel), damage(dmg), lifetime(5.0f), fromPlayer(player) {}
};

// Particle effect
struct Particle {
    Vector3 position;
    Vector3 velocity;
    float r, g, b, a;
    float size;
    float lifetime;
    int type; // 0=blood, 1=muzzle flash, 2=impact, 3=smoke

    Particle() : r(1), g(1), b(1), a(1), size(1), lifetime(1), type(0) {}
};

// Game world
class GameWorld {
public:
    static constexpr int WORLD_SIZE = 200;
    static constexpr int NAV_GRID_SIZE = 20; // Grid divisions for navmesh

    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<Zombie>> zombies;
    std::vector<std::unique_ptr<BaseBuilding>> buildings;
    std::vector<Bullet> bullets;
    std::vector<Particle> particles;
    std::vector<std::unique_ptr<NavMeshNode>> navMesh;

    int wave;
    int zombiesRemaining;
    int zombiesThisWave;
    float waveTimer;
    bool waveActive;

    float terrainHeights[256][256]; // Heightmap for terrain

    GameWorld();
    void initialize();
    void update(float deltaTime);
    void spawnZombie();
    void startNextWave();
    void generateTerrain();
    void generateNavMesh();
    float getTerrainHeight(float x, float y) const;
    bool isWalkable(float x, float y) const;
    NavMeshNode* getClosestNavNode(Vector3 pos) const;
    void addParticle(Particle p);
    void addBloodSplatter(Vector3 pos);
    void addMuzzleFlash(Vector3 pos, Vector3 dir);
};

#endif // ZOMBIE_GAME_H
