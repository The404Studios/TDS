#include "ZombieGame.h"
#include <algorithm>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>

// Weapon Implementation
Weapon::Weapon(WeaponType t) : type(t), lastFireTime(0), currentReloadTime(0), reloading(false) {
    switch (type) {
    case WeaponType::PISTOL:
        name = "Pistol";
        damage = 25.0f;
        fireRate = 0.3f;
        range = 50.0f;
        accuracy = 0.95f;
        clipSize = 12;
        maxAmmo = 120;
        reloadTime = 1.5f;
        break;
    case WeaponType::SHOTGUN:
        name = "Shotgun";
        damage = 60.0f;
        fireRate = 1.0f;
        range = 20.0f;
        accuracy = 0.7f;
        clipSize = 6;
        maxAmmo = 48;
        reloadTime = 2.5f;
        break;
    case WeaponType::RIFLE:
        name = "Rifle";
        damage = 40.0f;
        fireRate = 0.15f;
        range = 80.0f;
        accuracy = 0.9f;
        clipSize = 30;
        maxAmmo = 180;
        reloadTime = 2.0f;
        break;
    case WeaponType::MACHINE_GUN:
        name = "Machine Gun";
        damage = 20.0f;
        fireRate = 0.08f;
        range = 60.0f;
        accuracy = 0.75f;
        clipSize = 100;
        maxAmmo = 400;
        reloadTime = 4.0f;
        break;
    case WeaponType::SNIPER:
        name = "Sniper Rifle";
        damage = 150.0f;
        fireRate = 1.5f;
        range = 150.0f;
        accuracy = 0.98f;
        clipSize = 5;
        maxAmmo = 30;
        reloadTime = 3.0f;
        break;
    }
    currentClip = clipSize;
    ammo = maxAmmo;
}

bool Weapon::canFire() const {
    return !reloading && currentClip > 0;
}

void Weapon::fire() {
    if (canFire()) {
        currentClip--;
        lastFireTime = 0;
    }
}

void Weapon::reload() {
    if (!reloading && currentClip < clipSize && ammo > 0) {
        reloading = true;
        currentReloadTime = 0;
    }
}

void Weapon::update(float deltaTime) {
    lastFireTime += deltaTime;

    if (reloading) {
        currentReloadTime += deltaTime;
        if (currentReloadTime >= reloadTime) {
            int needed = clipSize - currentClip;
            int toReload = std::min(needed, ammo);
            currentClip += toReload;
            ammo -= toReload;
            reloading = false;
            currentReloadTime = 0;
        }
    }
}

// Player Implementation
Player::Player()
    : position(100, 100, 2), velocity(0, 0, 0), yaw(0), pitch(0),
    health(100), maxHealth(100), speed(15.0f), sprintMultiplier(1.5f),
    sprinting(false), money(0), kills(0), currentWeaponIndex(0) {

    forward = Vector3(0, 1, 0);
    right = Vector3(1, 0, 0);
    up = Vector3(0, 0, 1);

    // Give starting weapons
    weapons.push_back(std::make_unique<Weapon>(WeaponType::PISTOL));
    weapons.push_back(std::make_unique<Weapon>(WeaponType::SHOTGUN));
}

void Player::update(float deltaTime) {
    position = position + velocity * deltaTime;
    velocity = velocity * 0.8f; // Friction

    // Keep on terrain (simplified)
    if (position.z < 2.0f) position.z = 2.0f;

    // Bounds
    position.x = std::max(5.0f, std::min((float)GameWorld::WORLD_SIZE - 5.0f, position.x));
    position.y = std::max(5.0f, std::min((float)GameWorld::WORLD_SIZE - 5.0f, position.y));

    // Update current weapon
    if (getCurrentWeapon()) {
        getCurrentWeapon()->update(deltaTime);
    }
}

void Player::move(Vector3 direction, float deltaTime) {
    float moveSpeed = speed * (sprinting ? sprintMultiplier : 1.0f);
    Vector3 movement = direction.normalize() * moveSpeed;
    velocity = velocity + movement;
}

void Player::rotate(float deltaYaw, float deltaPitch) {
    yaw += deltaYaw;
    pitch += deltaPitch;

    // Clamp pitch
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateVectors();
}

void Player::updateVectors() {
    // Calculate forward vector
    float yawRad = yaw * 3.14159f / 180.0f;
    float pitchRad = pitch * 3.14159f / 180.0f;

    forward.x = cos(pitchRad) * sin(yawRad);
    forward.y = cos(pitchRad) * cos(yawRad);
    forward.z = sin(pitchRad);
    forward = forward.normalize();

    // Calculate right vector
    Vector3 worldUp(0, 0, 1);
    right = forward.cross(worldUp).normalize();

    // Calculate up vector
    up = right.cross(forward).normalize();
}

Weapon* Player::getCurrentWeapon() {
    if (currentWeaponIndex >= 0 && currentWeaponIndex < (int)weapons.size()) {
        return weapons[currentWeaponIndex].get();
    }
    return nullptr;
}

void Player::switchWeapon(int index) {
    if (index >= 0 && index < (int)weapons.size()) {
        currentWeaponIndex = index;
    }
}

void Player::takeDamage(float amount) {
    health -= amount;
    if (health < 0) health = 0;
}

// Zombie Implementation
Zombie::Zombie(Vector3 startPos)
    : position(startPos), velocity(0, 0, 0), health(100), maxHealth(100),
    speed(8.0f), damage(10.0f), attackRange(2.0f), attackCooldown(1.0f),
    lastAttackTime(0), isDead(false), currentPathNode(0), walkCycle(0), deathTimer(0) {
}

void Zombie::update(float deltaTime, Player* player, const std::vector<NavMeshNode*>& navMesh) {
    if (isDead) {
        deathTimer += deltaTime;
        return;
    }

    lastAttackTime += deltaTime;
    walkCycle += deltaTime * speed;

    // Find path to player periodically
    if (path.empty() || (rand() % 60 == 0)) {
        findPath(position, player->position, navMesh);
    }

    // Follow path
    followPath(deltaTime);

    // Attack if close
    float distToPlayer = (position - player->position).length();
    if (distToPlayer <= attackRange) {
        attack(player, deltaTime);
        velocity = Vector3(0, 0, 0); // Stop moving when attacking
    }

    // Apply movement
    position = position + velocity * deltaTime;
    velocity = velocity * 0.9f; // Friction

    // Keep on ground
    position.z = 0;
}

void Zombie::findPath(Vector3 start, Vector3 end, const std::vector<NavMeshNode*>& navMesh) {
    // Simple A* pathfinding
    path.clear();

    if (navMesh.empty()) {
        targetPosition = end;
        return;
    }

    // Find closest nodes
    NavMeshNode* startNode = nullptr;
    NavMeshNode* endNode = nullptr;
    float minStartDist = 1000000.0f;
    float minEndDist = 1000000.0f;

    for (auto* node : navMesh) {
        float startDist = (node->position - start).length();
        float endDist = (node->position - end).length();

        if (startDist < minStartDist) {
            minStartDist = startDist;
            startNode = node;
        }
        if (endDist < minEndDist) {
            minEndDist = endDist;
            endNode = node;
        }
    }

    if (!startNode || !endNode || !startNode->walkable || !endNode->walkable) {
        targetPosition = end;
        return;
    }

    // Reset all nodes
    for (auto* node : navMesh) {
        node->gCost = 10000000.0f;
        node->hCost = 0;
        node->parent = nullptr;
    }

    // A* algorithm
    std::vector<NavMeshNode*> openSet;
    std::vector<NavMeshNode*> closedSet;

    startNode->gCost = 0;
    startNode->hCost = (endNode->position - startNode->position).length();
    openSet.push_back(startNode);

    while (!openSet.empty()) {
        // Find node with lowest fCost
        NavMeshNode* current = openSet[0];
        int currentIndex = 0;
        for (int i = 1; i < (int)openSet.size(); i++) {
            if (openSet[i]->fCost() < current->fCost()) {
                current = openSet[i];
                currentIndex = i;
            }
        }

        // Found path
        if (current == endNode) {
            // Reconstruct path
            NavMeshNode* node = endNode;
            while (node != nullptr) {
                path.insert(path.begin(), node);
                node = node->parent;
            }
            currentPathNode = 0;
            return;
        }

        // Move current to closed set
        openSet.erase(openSet.begin() + currentIndex);
        closedSet.push_back(current);

        // Check neighbors
        for (NavMeshNode* neighbor : current->neighbors) {
            if (!neighbor->walkable) continue;

            bool inClosed = false;
            for (auto* closed : closedSet) {
                if (closed == neighbor) {
                    inClosed = true;
                    break;
                }
            }
            if (inClosed) continue;

            float tentativeG = current->gCost + (neighbor->position - current->position).length();

            bool inOpen = false;
            for (auto* open : openSet) {
                if (open == neighbor) {
                    inOpen = true;
                    break;
                }
            }

            if (!inOpen) {
                openSet.push_back(neighbor);
            }
            else if (tentativeG >= neighbor->gCost) {
                continue;
            }

            neighbor->parent = current;
            neighbor->gCost = tentativeG;
            neighbor->hCost = (endNode->position - neighbor->position).length();
        }
    }

    // No path found, move directly
    targetPosition = end;
}

void Zombie::followPath(float deltaTime) {
    if (!path.empty() && currentPathNode < (int)path.size()) {
        Vector3 targetPos = path[currentPathNode]->position;
        Vector3 direction = (targetPos - position).normalize();

        velocity = direction * speed;

        // Move to next node if close
        if ((position - targetPos).length() < 2.0f) {
            currentPathNode++;
        }
    }
    else if (targetPosition.length() > 0) {
        // No path, move directly
        Vector3 direction = (targetPosition - position).normalize();
        velocity = direction * speed;
    }
}

void Zombie::attack(Player* player, float deltaTime) {
    if (lastAttackTime >= attackCooldown) {
        player->takeDamage(damage);
        lastAttackTime = 0;
    }
}

void Zombie::takeDamage(float amount) {
    health -= amount;
    if (health <= 0) {
        health = 0;
        isDead = true;
    }
}

// Base Building Implementation
BaseBuilding::BaseBuilding(BuildingType t, Vector3 pos)
    : type(t), position(pos), level(1), active(true),
    fireRange(30.0f), fireRate(1.0f), lastFireTime(0), turretYaw(0) {

    switch (type) {
    case BuildingType::WALL:
        maxHealth = 500;
        cost = 100;
        break;
    case BuildingType::TURRET:
        maxHealth = 200;
        cost = 250;
        fireRange = 40.0f;
        break;
    case BuildingType::AMMO_STATION:
        maxHealth = 150;
        cost = 150;
        break;
    case BuildingType::HEALTH_STATION:
        maxHealth = 150;
        cost = 150;
        break;
    case BuildingType::BARRICADE:
        maxHealth = 300;
        cost = 75;
        break;
    }
    health = maxHealth;
}

void BaseBuilding::update(float deltaTime, std::vector<std::unique_ptr<Zombie>>& zombies) {
    if (!active || type != BuildingType::TURRET) return;

    lastFireTime += deltaTime;

    // Find closest zombie in range
    Zombie* target = nullptr;
    float closestDist = fireRange;

    for (auto& zombie : zombies) {
        if (zombie->isDead) continue;

        float dist = (zombie->position - position).length();
        if (dist < closestDist) {
            closestDist = dist;
            target = zombie.get();
        }
    }

    // Fire at target
    if (target && lastFireTime >= fireRate) {
        target->takeDamage(15.0f * level);
        lastFireTime = 0;

        // Update turret rotation
        Vector3 toTarget = (target->position - position).normalize();
        turretYaw = atan2(toTarget.x, toTarget.y) * 180.0f / 3.14159f;
    }
}

void BaseBuilding::upgrade() {
    level++;
    maxHealth *= 1.5f;
    health = maxHealth;

    if (type == BuildingType::TURRET) {
        fireRange *= 1.2f;
        fireRate *= 0.9f; // Faster
    }
}

void BaseBuilding::takeDamage(float amount) {
    health -= amount;
    if (health <= 0) {
        health = 0;
        active = false;
    }
}

const char* BaseBuilding::getName() const {
    switch (type) {
    case BuildingType::WALL: return "Wall";
    case BuildingType::TURRET: return "Turret";
    case BuildingType::AMMO_STATION: return "Ammo Station";
    case BuildingType::HEALTH_STATION: return "Health Station";
    case BuildingType::BARRICADE: return "Barricade";
    default: return "Unknown";
    }
}

int BaseBuilding::getUpgradeCost() const {
    return cost * level;
}

// GameWorld Implementation
GameWorld::GameWorld()
    : wave(0), zombiesRemaining(0), zombiesThisWave(0), waveTimer(0), waveActive(false) {
}

void GameWorld::initialize() {
    player = std::make_unique<Player>();
    generateTerrain();
    generateNavMesh();

    // Start first wave
    startNextWave();
}

void GameWorld::update(float deltaTime) {
    // Update player
    player->update(deltaTime);

    // Update zombies
    std::vector<NavMeshNode*> navMeshPtrs;
    for (auto& node : navMesh) {
        navMeshPtrs.push_back(node.get());
    }

    for (auto& zombie : zombies) {
        zombie->update(deltaTime, player.get(), navMeshPtrs);
    }

    // Update buildings
    for (auto& building : buildings) {
        building->update(deltaTime, zombies);
    }

    // Update bullets
    for (auto it = bullets.begin(); it != bullets.end();) {
        it->position = it->position + it->velocity * deltaTime;
        it->lifetime -= deltaTime;

        bool hit = false;

        // Check zombie collisions
        if (it->fromPlayer) {
            for (auto& zombie : zombies) {
                if (!zombie->isDead) {
                    float dist = (zombie->position - it->position).length();
                    if (dist < 1.5f) {
                        zombie->takeDamage(it->damage);
                        if (zombie->isDead) {
                            player->kills++;
                            player->money += 10 * wave;
                            addBloodSplatter(zombie->position);
                        }
                        hit = true;
                        break;
                    }
                }
            }
        }

        if (hit || it->lifetime <= 0) {
            it = bullets.erase(it);
        }
        else {
            ++it;
        }
    }

    // Update particles
    for (auto it = particles.begin(); it != particles.end();) {
        it->lifetime -= deltaTime;
        it->position = it->position + it->velocity * deltaTime;
        it->velocity.z -= 9.8f * deltaTime; // Gravity
        it->a = it->lifetime;

        if (it->lifetime <= 0) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }

    // Remove dead zombies after delay
    zombies.erase(std::remove_if(zombies.begin(), zombies.end(),
        [](const std::unique_ptr<Zombie>& z) { return z->isDead && z->deathTimer > 5.0f; }),
        zombies.end());

    // Wave management
    if (waveActive) {
        waveTimer += deltaTime;

        // Spawn zombies periodically
        if (zombiesRemaining > 0 && waveTimer > 2.0f) {
            spawnZombie();
            zombiesRemaining--;
            waveTimer = 0;
        }

        // Check if wave complete
        int aliveZombies = 0;
        for (auto& z : zombies) {
            if (!z->isDead) aliveZombies++;
        }

        if (zombiesRemaining == 0 && aliveZombies == 0) {
            waveActive = false;
            // Auto-start next wave after delay
            if (wave < 100) { // Prevent infinite waves
                // Wave will be started by pressing a key or automatically
            }
        }
    }
}

void GameWorld::spawnZombie() {
    // Spawn at random edge of map
    Vector3 spawnPos;
    int edge = rand() % 4;

    switch (edge) {
    case 0: spawnPos = Vector3(rand() % WORLD_SIZE, 0, 0); break;
    case 1: spawnPos = Vector3(rand() % WORLD_SIZE, WORLD_SIZE, 0); break;
    case 2: spawnPos = Vector3(0, rand() % WORLD_SIZE, 0); break;
    case 3: spawnPos = Vector3(WORLD_SIZE, rand() % WORLD_SIZE, 0); break;
    }

    auto zombie = std::make_unique<Zombie>(spawnPos);
    zombie->health += wave * 20; // Zombies get tougher
    zombie->maxHealth = zombie->health;
    zombie->speed += wave * 0.5f;
    zombie->damage += wave * 2.0f;

    zombies.push_back(std::move(zombie));
}

void GameWorld::startNextWave() {
    wave++;
    zombiesThisWave = 5 + wave * 5; // More zombies each wave
    zombiesRemaining = zombiesThisWave;
    waveTimer = 0;
    waveActive = true;

    std::cout << "Wave " << wave << " starting! Zombies: " << zombiesThisWave << std::endl;
}

void GameWorld::generateTerrain() {
    // Simple perlin-like terrain
    for (int x = 0; x < 256; x++) {
        for (int y = 0; y < 256; y++) {
            terrainHeights[x][y] = 0;
        }
    }
}

void GameWorld::generateNavMesh() {
    navMesh.clear();

    // Create grid of nav nodes
    for (int x = 0; x < NAV_GRID_SIZE; x++) {
        for (int y = 0; y < NAV_GRID_SIZE; y++) {
            float posX = (x * WORLD_SIZE) / (float)NAV_GRID_SIZE;
            float posY = (y * WORLD_SIZE) / (float)NAV_GRID_SIZE;

            auto node = std::make_unique<NavMeshNode>();
            node->position = Vector3(posX, posY, 0);
            node->walkable = isWalkable(posX, posY);
            navMesh.push_back(std::move(node));
        }
    }

    // Connect neighbors
    for (int x = 0; x < NAV_GRID_SIZE; x++) {
        for (int y = 0; y < NAV_GRID_SIZE; y++) {
            int idx = y * NAV_GRID_SIZE + x;
            NavMeshNode* node = navMesh[idx].get();

            // 8-directional connections
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;

                    int nx = x + dx;
                    int ny = y + dy;

                    if (nx >= 0 && nx < NAV_GRID_SIZE && ny >= 0 && ny < NAV_GRID_SIZE) {
                        int nidx = ny * NAV_GRID_SIZE + nx;
                        node->neighbors.push_back(navMesh[nidx].get());
                    }
                }
            }
        }
    }

    std::cout << "NavMesh generated with " << navMesh.size() << " nodes" << std::endl;
}

float GameWorld::getTerrainHeight(float x, float y) const {
    return 0; // Flat terrain for now
}

bool GameWorld::isWalkable(float x, float y) const {
    // Check if position is within bounds and not blocked by buildings
    if (x < 0 || x >= WORLD_SIZE || y < 0 || y >= WORLD_SIZE) {
        return false;
    }
    return true;
}

NavMeshNode* GameWorld::getClosestNavNode(Vector3 pos) const {
    NavMeshNode* closest = nullptr;
    float minDist = 100000.0f;

    for (auto& node : navMesh) {
        float dist = (node->position - pos).length();
        if (dist < minDist) {
            minDist = dist;
            closest = node.get();
        }
    }

    return closest;
}

void GameWorld::addParticle(Particle p) {
    particles.push_back(p);
}

void GameWorld::addBloodSplatter(Vector3 pos) {
    for (int i = 0; i < 20; i++) {
        Particle p;
        p.position = pos + Vector3(0, 0, 1);
        p.velocity = Vector3(
            (rand() % 100 - 50) / 10.0f,
            (rand() % 100 - 50) / 10.0f,
            (rand() % 100) / 10.0f
        );
        p.r = 0.8f + (rand() % 20) / 100.0f;
        p.g = 0.0f;
        p.b = 0.0f;
        p.a = 1.0f;
        p.size = 2.0f + (rand() % 20) / 10.0f;
        p.lifetime = 2.0f;
        p.type = 0; // Blood
        particles.push_back(p);
    }
}

void GameWorld::addMuzzleFlash(Vector3 pos, Vector3 dir) {
    for (int i = 0; i < 10; i++) {
        Particle p;
        p.position = pos;
        p.velocity = dir * (5.0f + rand() % 10) + Vector3(
            (rand() % 100 - 50) / 50.0f,
            (rand() % 100 - 50) / 50.0f,
            (rand() % 100 - 50) / 50.0f
        );
        p.r = 1.0f;
        p.g = 0.8f + (rand() % 20) / 100.0f;
        p.b = 0.3f;
        p.a = 1.0f;
        p.size = 1.5f;
        p.lifetime = 0.15f;
        p.type = 1; // Muzzle flash
        particles.push_back(p);
    }
}
