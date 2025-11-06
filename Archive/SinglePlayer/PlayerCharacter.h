#ifndef PLAYER_CHARACTER_H
#define PLAYER_CHARACTER_H

#include "CivilizationAI.h"
#include "TerrainManager.h"
#include <vector>

// Stick figure bone structure
struct Bone {
    Vector3 start;
    Vector3 end;
    float length;

    Bone() : length(0) {}
    Bone(const Vector3& s, const Vector3& e) : start(s), end(e) {
        Vector3 diff = e - s;
        length = diff.length();
    }
};

// IK (Inverse Kinematics) solver for legs
class IKChain {
public:
    std::vector<Bone> bones;
    Vector3 target;
    int iterations;

    IKChain() : iterations(10) {}

    void addBone(const Bone& bone) {
        bones.push_back(bone);
    }

    void solve();
    void solveBackward();
    void solveForward(const Vector3& rootPosition);
};

// Weapon data
struct Weapon {
    enum Type {
        PISTOL,
        RIFLE,
        SHOTGUN,
        SMG,
        SNIPER
    };

    Type type;
    std::string name;
    int maxAmmo;
    int currentAmmo;
    int magazineSize;
    float damage;
    float fireRate; // Shots per second
    float reloadTime;
    float range;
    float accuracy;
    bool isAutomatic;

    float timeSinceLastShot;
    bool isReloading;
    float reloadTimer;

    Weapon();
    Weapon(Type t);

    bool canShoot() const;
    void shoot();
    void reload();
    void update(float deltaTime);
};

// First-person player character
class PlayerCharacter {
private:
    // Position and orientation
    Vector3 position;
    Vector3 velocity;
    float yaw;   // Horizontal rotation
    float pitch; // Vertical rotation (looking up/down)

    // Physical properties
    float height;
    float eyeHeight;
    float radius;
    float mass;

    // Movement
    float moveSpeed;
    float sprintSpeed;
    float jumpForce;
    bool isGrounded;
    bool isSprinting;
    bool isCrouching;

    // Stick figure body parts
    Vector3 headPosition;
    Vector3 torsoTop;
    Vector3 torsoBottom;
    Vector3 leftShoulderPos;
    Vector3 rightShoulderPos;

    // Legs with IK
    IKChain leftLeg;
    IKChain rightLeg;
    Vector3 leftFootTarget;
    Vector3 rightFootTarget;
    float stepHeight;
    float stepLength;
    bool leftFootPlanted;
    float walkCycle;

    // Arms
    Vector3 leftHandPos;
    Vector3 rightHandPos;

    // Weapons
    std::vector<Weapon> weapons;
    int currentWeaponIndex;

    // Camera
    Vector3 cameraOffset;
    float cameraFOV;
    float cameraNearPlane;
    float cameraFarPlane;

    // Stats
    float health;
    float maxHealth;
    float stamina;
    float maxStamina;

    // Terrain reference
    TerrainManager* terrain;

    // Private methods
    void updateBodyPositions();
    void updateLegIK(float deltaTime);
    void updateArms();
    void handleCollisions();

public:
    PlayerCharacter(const Vector3& startPos, TerrainManager* terrainRef);

    // Movement
    void moveForward(float amount);
    void moveRight(float amount);
    void jump();
    void sprint(bool enable);
    void crouch(bool enable);

    // Looking
    void look(float deltaYaw, float deltaPitch);

    // Combat
    void shoot();
    void reload();
    void switchWeapon(int index);
    void nextWeapon();
    void previousWeapon();

    // Update
    void update(float deltaTime);

    // Rendering
    void renderStickFigure() const;
    void renderWeapon() const;
    void renderCrosshair() const;

    // Getters
    Vector3 getPosition() const { return position; }
    Vector3 getCameraPosition() const { return position + cameraOffset; }
    Vector3 getForwardVector() const;
    Vector3 getRightVector() const;
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getHealth() const { return health; }
    float getMaxHealth() const { return maxHealth; }
    float getStamina() const { return stamina; }
    Weapon& getCurrentWeapon() { return weapons[currentWeaponIndex]; }
    const Weapon& getCurrentWeapon() const { return weapons[currentWeaponIndex]; }

    // Setters
    void setPosition(const Vector3& pos) { position = pos; }
    void takeDamage(float amount);
    void heal(float amount);
};

#endif // PLAYER_CHARACTER_H
