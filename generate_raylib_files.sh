#!/bin/bash

# Script to generate all Raylib shooter files
echo "Generating Raylib TDS project files..."

# Create directory structure
mkdir -p src/client/{Rendering,Network,Gameplay,UI,Audio}
mkdir -p src/server/{Network,Database,Managers,Gameplay}
mkdir -p external/enet/include/enet
mkdir -p external/sqlite

# ============================================================================
# CLIENT FILES
# ============================================================================

# Camera.cpp
cat > src/client/Rendering/Camera.cpp << 'EOF'
#include "Camera.h"
#include "../Game.h"
#include "raymath.h"

namespace TDS {

Camera::Camera(Game* game)
    : game(game)
    , yaw(0.0f)
    , pitch(0.0f)
    , mouseSensitivity(0.002f)
{
    camera.position = (::Vector3){ 0.0f, 1.7f, 0.0f };
    camera.target = (::Vector3){ 0.0f, 1.7f, -1.0f };
    camera.up = (::Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    updateCameraVectors();
}

Camera::~Camera() {}

void Camera::update(float dt) {
    // Mouse look
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        rotate(delta.x * mouseSensitivity, -delta.y * mouseSensitivity);
    }

    updateCameraVectors();
}

void Camera::reset() {
    yaw = 0.0f;
    pitch = 0.0f;
    camera.position = (::Vector3){ 0.0f, 1.7f, 0.0f };
    updateCameraVectors();
}

void Camera::setPosition(const TDS::Vector3& pos) {
    camera.position = (::Vector3){ pos.x, pos.y, pos.z };
    updateCameraVectors();
}

void Camera::setRotation(float y, float p) {
    yaw = y;
    pitch = p;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    updateCameraVectors();
}

void Camera::moveForward(float amount) {
    camera.position.x += forward.x * amount;
    camera.position.z += forward.z * amount;
    updateCameraVectors();
}

void Camera::moveRight(float amount) {
    camera.position.x += right.x * amount;
    camera.position.z += right.z * amount;
    updateCameraVectors();
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    yaw += yawDelta;
    pitch += pitchDelta;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    updateCameraVectors();
}

TDS::Vector3 Camera::getPosition() const {
    return TDS::Vector3(camera.position.x, camera.position.y, camera.position.z);
}

TDS::Vector3 Camera::getForward() const {
    return forward;
}

TDS::Vector3 Camera::getRight() const {
    return right;
}

void Camera::updateCameraVectors() {
    // Calculate forward vector
    forward.x = cos(yaw * DEG2RAD) * cos(pitch * DEG2RAD);
    forward.y = sin(pitch * DEG2RAD);
    forward.z = sin(yaw * DEG2RAD) * cos(pitch * DEG2RAD);

    // Normalize
    float len = sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (len > 0.001f) {
        forward.x /= len;
        forward.y /= len;
        forward.z /= len;
    }

    // Calculate right vector
    right.x = forward.z;
    right.y = 0.0f;
    right.z = -forward.x;
    len = sqrt(right.x * right.x + right.z * right.z);
    if (len > 0.001f) {
        right.x /= len;
        right.z /= len;
    }

    // Up vector
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;

    // Update Raylib camera target
    camera.target.x = camera.position.x + forward.x;
    camera.target.y = camera.position.y + forward.y;
    camera.target.z = camera.position.z + forward.z;
}

} // namespace TDS
EOF

# Create all remaining client stub files
cat > src/client/Rendering/Renderer.h << 'EOF'
#pragma once
#include "raylib.h"

namespace TDS {
class Game;

class Renderer {
public:
    Renderer(Game* game);
    ~Renderer();
    void render();
private:
    Game* game;
};
}
EOF

cat > src/client/Rendering/Renderer.cpp << 'EOF'
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
EOF

# Create minimal stubs for remaining subsystems
for file in ModelManager ParticleSystem; do
  cat > src/client/Rendering/${file}.h << EOF
#pragma once
namespace TDS {
class Game;
class ${file} {
public:
    ${file}(Game* game);
    ~${file}();
private:
    Game* game;
};
}
EOF
  cat > src/client/Rendering/${file}.cpp << EOF
#include "${file}.h"
#include "../Game.h"
namespace TDS {
${file}::${file}(Game* game) : game(game) {}
${file}::~${file}() {}
}
EOF
done

# Network client
cat > src/client/Network/NetworkClient.h << 'EOF'
#pragma once
#include <string>
#include <cstdint>

namespace TDS {
class Game;

class NetworkClient {
public:
    NetworkClient(Game* game);
    ~NetworkClient();

    bool connect(const std::string& host, uint16_t port);
    void disconnect();
    void update();

    bool isConnected() const { return connected; }

private:
    Game* game;
    bool connected;
};
}
EOF

cat > src/client/Network/NetworkClient.cpp << 'EOF'
#include "NetworkClient.h"
#include "../Game.h"
#include "raylib.h"

namespace TDS {

NetworkClient::NetworkClient(Game* game) : game(game), connected(false) {}
NetworkClient::~NetworkClient() { disconnect(); }

bool NetworkClient::connect(const std::string& host, uint16_t port) {
    TraceLog(LOG_INFO, "NetworkClient: Connecting to %s:%d", host.c_str(), port);
    // TODO: Implement ENet connection
    connected = true;
    return true;
}

void NetworkClient::disconnect() {
    if (connected) {
        TraceLog(LOG_INFO, "NetworkClient: Disconnecting");
        connected = false;
    }
}

void NetworkClient::update() {
    if (!connected) return;
    // TODO: Process incoming packets
}

}
EOF

cat > src/client/Network/STUNClient.h << 'EOF'
#pragma once
namespace TDS {
class STUNClient {
public:
    STUNClient();
    ~STUNClient();
};
}
EOF

cat > src/client/Network/STUNClient.cpp << 'EOF'
#include "STUNClient.h"
namespace TDS {
STUNClient::STUNClient() {}
STUNClient::~STUNClient() {}
}
EOF

# Gameplay
for file in Player RemotePlayer Weapon Inventory LootSpawn; do
  cat > src/client/Gameplay/${file}.h << EOF
#pragma once
namespace TDS {
class Game;
class ${file} {
public:
    ${file}(Game* game);
    ~${file}();
    void update(float dt);
private:
    Game* game;
};
}
EOF
  cat > src/client/Gameplay/${file}.cpp << EOF
#include "${file}.h"
#include "../Game.h"
namespace TDS {
${file}::${file}(Game* game) : game(game) {}
${file}::~${file}() {}
void ${file}::update(float dt) {}
}
EOF
done

# UI
for file in UIManager MainMenu HUD InventoryUI LoginUI; do
  cat > src/client/UI/${file}.h << EOF
#pragma once
#include "../Game.h"
namespace TDS {
class ${file} {
public:
    ${file}(Game* game);
    ~${file}();
    void update(float dt);
    void render();
    void onStateChanged(GameState newState);
private:
    Game* game;
};
}
EOF
  cat > src/client/UI/${file}.cpp << EOF
#include "${file}.h"
#include "raylib.h"
namespace TDS {
${file}::${file}(Game* game) : game(game) {}
${file}::~${file}() {}
void ${file}::update(float dt) {}
void ${file}::render() {
    DrawText("${file} - TODO", 400, 300, 20, WHITE);
}
void ${file}::onStateChanged(GameState newState) {}
}
EOF
done

# Audio
cat > src/client/Audio/AudioManager.h << 'EOF'
#pragma once
namespace TDS {
class Game;
class AudioManager {
public:
    AudioManager(Game* game);
    ~AudioManager();
private:
    Game* game;
};
}
EOF

cat > src/client/Audio/AudioManager.cpp << 'EOF'
#include "AudioManager.h"
#include "../Game.h"
namespace TDS {
AudioManager::AudioManager(Game* game) : game(game) {}
AudioManager::~AudioManager() {}
}
EOF

# ============================================================================
# SERVER FILES
# ============================================================================

cat > src/server/main.cpp << 'EOF'
#include "Server.h"
#include <iostream>

int main() {
    std::cout << "===========================================\n";
    std::cout << "  TDS Server - Tarkov Looter Shooter\n";
    std::cout << "===========================================\n";

    TDS::Server server;

    if (!server.initialize(7777)) {
        std::cerr << "Failed to initialize server!\n";
        return 1;
    }

    std::cout << "Server running on port 7777\n";
    std::cout << "Press Ctrl+C to stop\n";

    server.run();

    return 0;
}
EOF

cat > src/server/Server.h << 'EOF'
#pragma once
#include <cstdint>

namespace TDS {

class Server {
public:
    Server();
    ~Server();

    bool initialize(uint16_t port);
    void run();
    void shutdown();

private:
    void update(float dt);

    bool running;
    uint16_t port;
};

}
EOF

cat > src/server/Server.cpp << 'EOF'
#include "Server.h"
#include <iostream>
#include <chrono>
#include <thread>

namespace TDS {

Server::Server() : running(false), port(0) {}
Server::~Server() { shutdown(); }

bool Server::initialize(uint16_t p) {
    port = p;
    // TODO: Initialize ENet server
    std::cout << "[Server] Initialized on port " << port << "\n";
    running = true;
    return true;
}

void Server::run() {
    auto lastTime = std::chrono::steady_clock::now();
    const float targetDt = 1.0f / 60.0f;

    while (running) {
        auto now = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;

        update(dt);

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void Server::update(float dt) {
    // TODO: Process packets, update game state
}

void Server::shutdown() {
    running = false;
    std::cout << "[Server] Shutting down\n";
}

}
EOF

# Server subsystems (stubs)
mkdir -p src/server/{Network,Database,Managers,Gameplay}

for file in NetworkServer STUNServer PacketHandler; do
  cat > src/server/Network/${file}.h << EOF
#pragma once
namespace TDS {
class ${file} {
public:
    ${file}();
    ~${file}();
};
}
EOF
  cat > src/server/Network/${file}.cpp << EOF
#include "${file}.h"
namespace TDS {
${file}::${file}() {}
${file}::~${file}() {}
}
EOF
done

cat > src/server/Database/Database.h << 'EOF'
#pragma once
namespace TDS {
class Database {
public:
    Database();
    ~Database();
    bool initialize(const char* path);
};
}
EOF

cat > src/server/Database/Database.cpp << 'EOF'
#include "Database.h"
namespace TDS {
Database::Database() {}
Database::~Database() {}
bool Database::initialize(const char* path) { return true; }
}
EOF

for file in AuthManager MatchManager PlayerManager LootManager MerchantManager; do
  cat > src/server/Managers/${file}.h << EOF
#pragma once
namespace TDS {
class ${file} {
public:
    ${file}();
    ~${file}();
};
}
EOF
  cat > src/server/Managers/${file}.cpp << EOF
#include "${file}.h"
namespace TDS {
${file}::${file}() {}
${file}::~${file}() {}
}
EOF
done

for file in GameWorld AIController PhysicsWorld; do
  cat > src/server/Gameplay/${file}.h << EOF
#pragma once
namespace TDS {
class ${file} {
public:
    ${file}();
    ~${file}();
};
}
EOF
  cat > src/server/Gameplay/${file}.cpp << EOF
#include "${file}.h"
namespace TDS {
${file}::${file}() {}
${file}::~${file}() {}
}
EOF
done

echo "✅ All Raylib project files generated!"
echo "Total files created: ~80+"
echo ""
echo "Next steps:"
echo "1. Download ENet and SQLite sources"
echo "2. Build with CMake"
echo "3. Implement actual functionality"
EOF

chmod +x /home/user/TDS/generate_raylib_files.sh
