#include "Player.h"
#include "../Game.h"
#include "../Rendering/Camera.h"
#include "../Network/NetworkClient.h"
#include "../Audio/AudioManager.h"
#include "common/Math.h"
#include "common/Items.h"
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
    , isAiming(false)
    , isReloading(false)
    , currentWeaponIndex(0)
    , weaponSwayAmount(0.0f)
    , weaponBobTime(0.0f)
    , recoilTime(0.0f)
    , baseFOV(75.0f)
    , currentFOV(75.0f)
    , targetFOV(75.0f)
    , sprintFOV(85.0f)
    , adsFOV(55.0f)
    , adsTransitionSpeed(8.0f)
    , adsProgress(0.0f)
    , networkUpdateTimer(0.0f)
    , networkUpdateInterval(0.05f) // 20 Hz updates
{
    position = TDS::Vector3(0, 1.7f, 0);
    velocity = TDS::Vector3(0, 0, 0);

    // Initialize weapon offsets
    hipFireOffset = TDS::Vector3(0.3f, -0.15f, -0.5f);  // Right and slightly down
    adsOffset = TDS::Vector3(0.0f, -0.05f, -0.35f);     // Centered and closer
    weaponOffset = hipFireOffset;
    weaponTargetOffset = hipFireOffset;
    recoilOffset = TDS::Vector3(0, 0, 0);

    // Initialize with default weapon
    equippedWeapons.push_back(Items::AK74);
}

Player::~Player() {}

void Player::update(float dt) {
    handleInput(dt);
    updatePhysics(dt);
    updateWeaponMotion(dt);
    updateADS(dt);
    updateFOV(dt);
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

    // Aim Down Sights (hold right mouse button)
    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && !isSprinting && !isReloading) {
        isAiming = true;
    } else {
        isAiming = false;
    }

    // Shooting
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isReloading) {
        shoot();
    }

    // Reload
    if (IsKeyPressed(KEY_R) && !isReloading) {
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
            state.flags = (isCrouching ? 1 : 0) | (isSprinting ? 2 : 0) | (isAiming ? 4 : 0);

            game->getNetwork()->sendPlayerMove(state);
        }
    }
}

void Player::updateWeaponMotion(float dt) {
    // Calculate movement speed for weapon bob
    float moveSpeed = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);

    // Update weapon bob time
    if (moveSpeed > 0.1f && isGrounded) {
        weaponBobTime += dt * moveSpeed * 2.0f;
    } else {
        weaponBobTime = 0.0f;
    }

    // Calculate weapon bob offset
    TDS::Vector3 bobOffset(0, 0, 0);
    if (moveSpeed > 0.1f && !isAiming) {
        float bobFrequency = isSprinting ? 1.8f : 1.0f;
        bobOffset.x = sinf(weaponBobTime * bobFrequency) * 0.02f;
        bobOffset.y = fabsf(sinf(weaponBobTime * 2.0f * bobFrequency)) * 0.03f;
    }

    // Calculate weapon sway from mouse movement
    TDS::Vector3 swayOffset(0, 0, 0);
    if (!isAiming) {
        Vector2 mouseDelta = GetMouseDelta();
        float swayScale = 0.001f;
        swayOffset.x = -mouseDelta.x * swayScale;
        swayOffset.y = -mouseDelta.y * swayScale;

        // Clamp sway
        swayOffset.x = Clamp(swayOffset.x, -0.05f, 0.05f);
        swayOffset.y = Clamp(swayOffset.y, -0.05f, 0.05f);
    }

    // Update recoil recovery
    if (recoilTime > 0.0f) {
        recoilTime -= dt * 3.0f; // Recover over time
        if (recoilTime < 0.0f) recoilTime = 0.0f;
    }

    // Smooth recoil offset recovery
    recoilOffset.x = Lerp(recoilOffset.x, 0.0f, dt * 5.0f);
    recoilOffset.y = Lerp(recoilOffset.y, 0.0f, dt * 5.0f);
    recoilOffset.z = Lerp(recoilOffset.z, 0.0f, dt * 10.0f);

    // Combine all offsets
    TDS::Vector3 totalOffset = TDS::Math::add(weaponTargetOffset, bobOffset);
    totalOffset = TDS::Math::add(totalOffset, swayOffset);
    totalOffset = TDS::Math::add(totalOffset, recoilOffset);

    // Smoothly interpolate to target offset
    weaponOffset.x = Lerp(weaponOffset.x, totalOffset.x, dt * 10.0f);
    weaponOffset.y = Lerp(weaponOffset.y, totalOffset.y, dt * 10.0f);
    weaponOffset.z = Lerp(weaponOffset.z, totalOffset.z, dt * 10.0f);
}

void Player::updateADS(float dt) {
    // Smoothly transition between hip fire and ADS
    float targetProgress = isAiming ? 1.0f : 0.0f;
    adsProgress = Lerp(adsProgress, targetProgress, dt * adsTransitionSpeed);

    // Interpolate weapon position between hip and ADS
    weaponTargetOffset.x = Lerp(hipFireOffset.x, adsOffset.x, adsProgress);
    weaponTargetOffset.y = Lerp(hipFireOffset.y, adsOffset.y, adsProgress);
    weaponTargetOffset.z = Lerp(hipFireOffset.z, adsOffset.z, adsProgress);
}

void Player::updateFOV(float dt) {
    // Determine target FOV based on state
    if (isAiming) {
        targetFOV = adsFOV;
    } else if (isSprinting) {
        targetFOV = sprintFOV;
    } else {
        targetFOV = baseFOV;
    }

    // Smoothly interpolate FOV
    currentFOV = Lerp(currentFOV, targetFOV, dt * 8.0f);

    // Update camera FOV
    if (game->getCamera()) {
        game->getCamera()->setFOV(currentFOV);
    }
}

void Player::applyRecoil() {
    if (equippedWeapons.empty()) return;

    uint16_t weaponId = equippedWeapons[currentWeaponIndex];
    auto* weaponData = ItemDatabase::getWeapon(weaponId);
    if (!weaponData) return;

    // Apply weapon-specific recoil
    float recoilAmount = 0.05f; // Base recoil

    // Reduce recoil when aiming
    if (isAiming) {
        recoilAmount *= 0.4f;
    }

    // Apply recoil offset
    recoilOffset.y += recoilAmount * 0.5f;  // Vertical kick
    recoilOffset.z -= recoilAmount * 0.3f;  // Weapon pushback
    recoilOffset.x += (GetRandomValue(-100, 100) / 1000.0f) * recoilAmount; // Horizontal spread

    // Apply camera recoil (pitch up)
    if (game->getCamera()) {
        float cameraPitchRecoil = recoilAmount * 200.0f;
        if (isAiming) cameraPitchRecoil *= 0.5f;

        float currentPitch = game->getCamera()->getPitch();
        game->getCamera()->setPitch(currentPitch + cameraPitchRecoil);
    }

    recoilTime = 1.0f;
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

    // Apply recoil
    applyRecoil();

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

    // Play sound effect (if available)
    if (game->getAudio()) {
        // Try weapon-specific sound, fallback to generic
        if (weaponData->type == WeaponType::ASSAULT_RIFLE) {
            game->getAudio()->playSound("ak47_fire", 0.7f);
        } else if (weaponData->type == WeaponType::PISTOL) {
            game->getAudio()->playSound("pistol_fire", 0.6f);
        } else if (weaponData->type == WeaponType::SNIPER_RIFLE) {
            game->getAudio()->playSound("sniper_fire", 0.8f);
        }
    }

    // Visual effects
    // TODO: Muzzle flash
}

void Player::reload() {
    if (equippedWeapons.empty()) return;

    TraceLog(LOG_INFO, "Reloading weapon...");

    isReloading = true;

    // Play reload sound
    if (game->getAudio()) {
        game->getAudio()->playSound("reload", 0.5f);
    }

    // TODO: Implement actual reload animation and ammo system
    // For now, just simulate a 2-second reload time
    // In a real implementation, you would use a timer or animation callback
    isReloading = false; // Will be set to false after animation completes
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

bool Player::getIsAiming() const {
    return isAiming;
}

TDS::Vector3 Player::getWeaponOffset() const {
    return weaponOffset;
}

float Player::getCurrentFOV() const {
    return currentFOV;
}

} // namespace TDS
