#pragma once

#include "common/Protocol.h"
#include "common/Items.h"
#include <vector>

namespace TDS {

class Game;

class Player {
public:
    Player(Game* game);
    ~Player();

    void update(float dt);

    // Getters
    TDS::Vector3 getPosition() const;
    float getHealth() const;
    float getMaxHealth() const;
    bool getIsCrouching() const;
    bool getIsSprinting() const;
    uint16_t getCurrentWeapon() const;

    // Setters
    void setPosition(const TDS::Vector3& pos);

    // Actions
    void takeDamage(float amount);
    void heal(float amount);
    void shoot();
    void reload();
    void switchWeapon(int index);

private:
    void handleInput(float dt);
    void updatePhysics(float dt);
    void updateNetworkSync(float dt);
    void die();

    Game* game;

    // Transform
    TDS::Vector3 position;
    TDS::Vector3 velocity;

    // Stats
    float health;
    float maxHealth;
    float speed;
    float sprintMultiplier;
    float crouchMultiplier;
    float jumpForce;
    float gravity;

    // State
    bool isGrounded;
    bool isCrouching;
    bool isSprinting;

    // Weapons
    std::vector<uint16_t> equippedWeapons;
    int currentWeaponIndex;

    // Network
    float networkUpdateTimer;
    float networkUpdateInterval;
};

} // namespace TDS
