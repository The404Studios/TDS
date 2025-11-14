#include "Renderer.h"
#include "../Game.h"
#include "Camera.h"
#include "ModelManager.h"
#include "raylib.h"

namespace TDS {

Renderer::Renderer(Game* game) : game(game) {}

Renderer::~Renderer() {}

void Renderer::render() {
    if (!game || !game->getCamera()) return;

    BeginMode3D(game->getCamera()->getCamera3D());

    // Draw floor
    Model* floor = game->getModelManager()->getModel("floor");
    if (floor) {
        DrawModel(*floor, (::Vector3){0, 0, 0}, 1.0f, WHITE);
    } else {
        DrawGrid(100, 1.0f);
    }

    // Draw some test cubes
    Model* cube = game->getModelManager()->getModel("cube");
    if (cube) {
        DrawModel(*cube, (::Vector3){0, 0.5f, 0}, 1.0f, RED);
        DrawModel(*cube, (::Vector3){5, 0.5f, 0}, 1.0f, BLUE);
        DrawModel(*cube, (::Vector3){0, 0.5f, 5}, 1.0f, GREEN);
        DrawModel(*cube, (::Vector3){-5, 0.5f, 0}, 1.0f, YELLOW);
        DrawModel(*cube, (::Vector3){0, 0.5f, -5}, 1.0f, PURPLE);
    }

    // Draw weapon in hand (if available)
    if (game->getPlayer()) {
        drawWeapon();
    }

    // TODO: Draw other players
    // TODO: Draw loot items
    // TODO: Draw AI enemies

    EndMode3D();
}

void Renderer::drawWeapon() {
    // Get weapon model based on player's current weapon
    uint16_t weaponId = game->getPlayer()->getCurrentWeapon();
    Model* weapon = nullptr;

    // Try to get specific weapon model, fallback to generic
    if (weaponId == Items::AK74 || weaponId == Items::AK74M) {
        weapon = game->getModelManager()->getModel("ak74");
    } else if (weaponId == Items::M4A1) {
        weapon = game->getModelManager()->getModel("m4a1");
    } else if (weaponId == Items::GLOCK_17 || weaponId == Items::MAKAROV) {
        weapon = game->getModelManager()->getModel("glock");
    } else if (weaponId == Items::SVD) {
        weapon = game->getModelManager()->getModel("svd");
    }

    if (!weapon) {
        weapon = game->getModelManager()->getModel("weapon");
    }
    if (!weapon) return;

    // Position weapon relative to camera using player's weapon offset
    Camera* camera = game->getCamera();
    if (!camera) return;

    TDS::Vector3 camPos = camera->getPosition();
    TDS::Vector3 forward = camera->getForward();
    TDS::Vector3 right = camera->getRight();

    // Get weapon offset from player (includes ADS, bob, sway, recoil)
    TDS::Vector3 offset = game->getPlayer()->getWeaponOffset();

    // Apply offset relative to camera orientation
    ::Vector3 weaponPos = {
        camPos.x + right.x * offset.x + forward.x * (-offset.z),
        camPos.y + offset.y,
        camPos.z + right.z * offset.x + forward.z * (-offset.z)
    };

    // Calculate weapon rotation to match camera
    float yaw = camera->getYaw();
    float pitch = camera->getPitch();

    // Draw weapon with orientation
    // Note: Raylib DrawModelEx allows rotation
    Vector3 rotationAxis = {0, 1, 0};
    float rotationAngle = yaw;

    DrawModelEx(*weapon, weaponPos, rotationAxis, rotationAngle, (Vector3){0.3f, 0.3f, 0.3f}, DARKGRAY);
}

} // namespace TDS
