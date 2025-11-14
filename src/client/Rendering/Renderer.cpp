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
    // Get weapon model
    Model* weapon = game->getModelManager()->getModel("weapon");
    if (!weapon) return;

    // Position weapon relative to camera
    Camera* camera = game->getCamera();
    if (!camera) return;

    TDS::Vector3 camPos = camera->getPosition();
    TDS::Vector3 forward = camera->getForward();
    TDS::Vector3 right = camera->getRight();

    // Weapon offset (right and down from camera)
    ::Vector3 weaponPos = {
        camPos.x + right.x * 0.3f + forward.x * 0.5f,
        camPos.y - 0.3f + forward.y * 0.5f,
        camPos.z + right.z * 0.3f + forward.z * 0.5f
    };

    // Draw weapon
    DrawModel(*weapon, weaponPos, 0.5f, DARKGRAY);
}

} // namespace TDS
