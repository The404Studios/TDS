#include "Renderer.h"
#include "../Game.h"
#include "Camera.h"

namespace TDS {

Renderer::Renderer(Game* game) : game(game) {}
Renderer::~Renderer() {}

void Renderer::render() {
    if (!game || !game->getCamera()) return;

    BeginMode3D(game->getCamera()->getCamera3D());

    // Draw grid
    DrawGrid(100, 1.0f);

    // Draw simple scene
    DrawCube((::Vector3){0, 0.5f, 0}, 1, 1, 1, RED);
    DrawCube((::Vector3){5, 0.5f, 0}, 1, 1, 1, BLUE);
    DrawCube((::Vector3){0, 0.5f, 5}, 1, 1, 1, GREEN);

    EndMode3D();
}

}
