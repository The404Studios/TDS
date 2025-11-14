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
    bool getIsAiming() const;
    uint16_t getCurrentWeapon() const;
    TDS::Vector3 getWeaponOffset() const;
    float getCurrentFOV() const;

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
    void updateWeaponMotion(float dt);
    void updateADS(float dt);
    void updateFOV(float dt);
    void applyRecoil();
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
    bool isAiming;
    bool isReloading;

    // Weapons
    std::vector<uint16_t> equippedWeapons;
    int currentWeaponIndex;

    // Weapon motion and ADS
    TDS::Vector3 weaponOffset;         // Current weapon position offset
    TDS::Vector3 weaponTargetOffset;   // Target offset for smooth interpolation
    TDS::Vector3 hipFireOffset;        // Default hip fire position
    TDS::Vector3 adsOffset;            // Aim down sights position
    float weaponSwayAmount;            // Current sway magnitude
    float weaponBobTime;               // Time accumulator for head bob
    float recoilTime;                  // Time since last shot (for recoil recovery)
    TDS::Vector3 recoilOffset;         // Current recoil offset

    // FOV
    float baseFOV;
    float currentFOV;
    float targetFOV;
    float sprintFOV;
    float adsFOV;

    // ADS transition
    float adsTransitionSpeed;
    float adsProgress;                 // 0.0 = hip fire, 1.0 = fully aimed

    // Network
    float networkUpdateTimer;
    float networkUpdateInterval;
};

} // namespace TDS
