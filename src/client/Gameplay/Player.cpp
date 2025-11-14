#include "Player.h"
#include "../Game.h"
#include "Rendering/Camera.h"
#include "Network/NetworkClient.h"
#include "common/Math.h"
#include "raylib.h"
#include "raymath.h"

namespace TDS {

Player::Player(Game* game)
    : game(game)
    , health(100.0f)
    , maxHealth(100.0f)
    , speed(5.0f)
    , sprintMultiplier(1.8f)
    , crouchMultiplier(0.5f)
    , jumpForce(8.0f)
    , gravity(20.0f)
    , isGrounded(false)
    , isCrouching(false)
    , isSprinting(false)
    , currentWeaponIndex(0)
    , networkUpdateTimer(0.0f)
    , networkUpdateInterval(0.05f) // 20 Hz updates
{
    position = TDS::Vector3(0, 1.7f, 0);
    velocity = TDS::Vector3(0, 0, 0);

    // Initialize with default weapon
    equippedWeapons.push_back(Items::AK74);
}

Player::~Player() {}

void Player::update(float dt) {
    handleInput(dt);
    updatePhysics(dt);
    updateNetworkSync(dt);
}

void Player::handleInput(float dt) {
    if (!game || !game->getCamera()) return;

    Camera* camera = game->getCamera();

    // Movement input
    TDS::Vector3 moveDir(0, 0, 0);

    if (IsKeyDown(KEY_W)) {
        TDS::Vector3 forward = camera->getForward();
        forward.y = 0; // Flatten for ground movement
        moveDir = TDS::Math::add(moveDir, TDS::Math::normalize(forward));
    }
    if (IsKeyDown(KEY_S)) {
        TDS::Vector3 forward = camera->getForward();
        forward.y = 0;
        moveDir = TDS::Math::subtract(moveDir, TDS::Math::normalize(forward));
    }
    if (IsKeyDown(KEY_A)) {
        moveDir = TDS::Math::subtract(moveDir, camera->getRight());
    }
    if (IsKeyDown(KEY_D)) {
        moveDir = TDS::Math::add(moveDir, camera->getRight());
    }

    // Normalize movement direction
    if (TDS::Math::lengthSquared(moveDir) > 0.01f) {
        moveDir = TDS::Math::normalize(moveDir);
    }

    // Apply movement speed modifiers
    float currentSpeed = speed;
    isSprinting = IsKeyDown(KEY_LEFT_SHIFT) && !isCrouching;

    if (isSprinting) {
        currentSpeed *= sprintMultiplier;
    }
    if (isCrouching) {
        currentSpeed *= crouchMultiplier;
    }

    // Apply horizontal movement
    velocity.x = moveDir.x * currentSpeed;
    velocity.z = moveDir.z * currentSpeed;

    // Crouch toggle
    if (IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_C)) {
        isCrouching = !isCrouching;

        if (isCrouching) {
            position.y -= 0.5f; // Lower stance
        } else {
            position.y += 0.5f; // Return to standing
        }
    }

    // Jump
    if (IsKeyPressed(KEY_SPACE) && isGrounded && !isCrouching) {
        velocity.y = jumpForce;
        isGrounded = false;
    }

    // Weapon switching
    if (IsKeyPressed(KEY_ONE)) switchWeapon(0);
    if (IsKeyPressed(KEY_TWO)) switchWeapon(1);
    if (IsKeyPressed(KEY_THREE)) switchWeapon(2);

    // Scroll wheel weapon switch
    float wheel = GetMouseWheelMove();
    if (wheel > 0) {
        switchWeapon((currentWeaponIndex + 1) % equippedWeapons.size());
    } else if (wheel < 0) {
        switchWeapon((currentWeaponIndex - 1 + equippedWeapons.size()) % equippedWeapons.size());
    }

    // Shooting
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        shoot();
    }

    // Reload
    if (IsKeyPressed(KEY_R)) {
        reload();
    }
}

void Player::updatePhysics(float dt) {
    // Apply gravity
    if (!isGrounded) {
        velocity.y -= gravity * dt;
    }

    // Update position
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    position.z += velocity.z * dt;

    // Simple ground collision (Y = 1.7 for standing, 1.2 for crouching)
    float groundHeight = isCrouching ? 1.2f : 1.7f;
    if (position.y <= groundHeight) {
        position.y = groundHeight;
        velocity.y = 0;
        isGrounded = true;
    } else {
        isGrounded = false;
    }

    // Update camera position
    if (game->getCamera()) {
        game->getCamera()->setPosition(position);
    }
}

void Player::updateNetworkSync(float dt) {
    networkUpdateTimer += dt;

    if (networkUpdateTimer >= networkUpdateInterval) {
        networkUpdateTimer = 0.0f;

        // Send position update to server
        if (game->getNetwork() && game->getNetwork()->isConnected()) {
            PlayerState state;
            state.playerId = game->getPlayerId();
            state.position = position;
            state.velocity = velocity;
            state.yaw = game->getCamera()->getYaw();
            state.pitch = game->getCamera()->getPitch();
            state.weaponIndex = currentWeaponIndex;
            state.health = static_cast<uint16_t>(health);
            state.maxHealth = static_cast<uint16_t>(maxHealth);
            state.flags = (isCrouching ? 1 : 0) | (isSprinting ? 2 : 0);

            game->getNetwork()->sendPlayerMove(state);
        }
    }
}

void Player::shoot() {
    if (equippedWeapons.empty()) return;

    uint16_t weaponId = equippedWeapons[currentWeaponIndex];
    auto* weaponData = ItemDatabase::getWeapon(weaponId);

    if (!weaponData) return;

    TraceLog(LOG_INFO, "Firing weapon: %d", weaponId);

    // Get camera for aim direction
    Camera* camera = game->getCamera();
    if (!camera) return;

    TDS::Vector3 forward = camera->getForward();

    // Create raycast from camera
    TDS::Math::Ray ray(position, forward);

    // Perform raycast (simplified - no actual hit detection yet)
    // TODO: Check against world geometry and other players

    // Send fire event to server
    if (game->getNetwork() && game->getNetwork()->isConnected()) {
        WeaponFireEvent fireEvent;
        fireEvent.playerId = game->getPlayerId();
        fireEvent.origin = position;
        fireEvent.direction = forward;
        fireEvent.weaponId = weaponId;
        fireEvent.timestamp = static_cast<uint32_t>(GetTime() * 1000);

        game->getNetwork()->sendWeaponFire(fireEvent);
    }

    // Play sound effect
    // TODO: game->getAudio()->playSound(weaponData->fireSound);

    // Visual effects
    // TODO: Muzzle flash, recoil, etc.
}

void Player::reload() {
    TraceLog(LOG_INFO, "Reloading weapon...");
    // TODO: Implement reload logic with animation
}

void Player::switchWeapon(int index) {
    if (index < 0 || index >= equippedWeapons.size()) return;

    if (currentWeaponIndex != index) {
        currentWeaponIndex = index;
        TraceLog(LOG_INFO, "Switched to weapon: %d", equippedWeapons[currentWeaponIndex]);
    }
}

void Player::takeDamage(float amount) {
    health -= amount;
    if (health < 0) health = 0;

    TraceLog(LOG_WARNING, "Took %.0f damage! Health: %.0f/%.0f", amount, health, maxHealth);

    if (health <= 0) {
        die();
    }
}

void Player::heal(float amount) {
    health += amount;
    if (health > maxHealth) health = maxHealth;

    TraceLog(LOG_INFO, "Healed %.0f HP! Health: %.0f/%.0f", amount, health, maxHealth);
}

void Player::die() {
    TraceLog(LOG_ERROR, "Player died!");
    // TODO: Death screen, respawn logic
}

TDS::Vector3 Player::getPosition() const {
    return position;
}

void Player::setPosition(const TDS::Vector3& pos) {
    position = pos;
}

float Player::getHealth() const {
    return health;
}

float Player::getMaxHealth() const {
    return maxHealth;
}

bool Player::getIsCrouching() const {
    return isCrouching;
}

bool Player::getIsSprinting() const {
    return isSprinting;
}

uint16_t Player::getCurrentWeapon() const {
    if (equippedWeapons.empty()) return 0;
    return equippedWeapons[currentWeaponIndex];
}

} // namespace TDS
