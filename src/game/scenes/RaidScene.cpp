#include "RaidScene.h"
#include "../../engine/GameEngine.h"
#include "../../common/NetworkProtocol.h"
#include "../../common/ItemDatabase.h"
#include <iostream>

RaidScene::RaidScene(NetworkClient* netClient, uint64_t playerAccId)
    : networkClient(netClient),
      playerAccountId(playerAccId),
      nextEntityId(1),
      playerEntityId(INVALID_ENTITY),
      mouseX(0.0f),
      mouseY(0.0f),
      moveForward(false),
      moveBackward(false),
      moveLeft(false),
      moveRight(false) {
}

RaidScene::~RaidScene() {
}

bool RaidScene::onEnter() {
    std::cout << "[RaidScene] Entering raid" << std::endl;

    // Initialize prefabs from ItemDatabase
    auto& itemDb = ItemDatabase::getInstance();
    for (const auto& pair : itemDb.getAllItems()) {
        const Item& item = pair.second;
        prefabs[item.id] = Prefab::fromItem(item);
    }

    // Add special prefabs
    prefabs["player"] = Prefab::player();
    prefabs["scav"] = Prefab::scav();
    prefabs["loot_crate"] = Prefab::lootCrate();

    // Load map
    loadMap();

    // Spawn player
    spawnPlayer();

    // Spawn AI
    spawnAI();

    // Spawn loot
    spawnLoot();

    // Initialize weather
    auto weather = ENGINE.getWeatherSystem();
    if (weather) {
        weather->setWeather(WeatherType::CLEAR);
        weather->getTimeOfDay().hour = 12.0f;  // Noon
    }

    // Register network callbacks
    // TODO: Set up network packet handlers for PLAYER_MOVE, PLAYER_SHOOT, etc.

    std::cout << "[RaidScene] Raid initialized with " << entities.size() << " entities" << std::endl;
    return true;
}

void RaidScene::onExit() {
    std::cout << "[RaidScene] Exiting raid" << std::endl;

    // Cleanup all entities
    entities.clear();
    prefabs.clear();

    // Unregister network callbacks
}

void RaidScene::fixedUpdate(float dt) {
    // Physics simulation
    updatePhysics(dt);

    // Network state sync (if authoritative)
    // TODO: Send PLAYER_MOVE packets
}

void RaidScene::update(float dt) {
    // Process network packets
    processNetworkPackets();

    // Update player
    updatePlayer(dt);

    // Update AI
    updateAI(dt);

    // Update audio
    auto audio = ENGINE.getAudioEngine();
    if (audio) {
        audio->setListenerPosition(cameraPosition);
        audio->setListenerOrientation(cameraRotation);
    }

    // Update particles
    auto particles = ENGINE.getParticleSystem();
    if (particles) {
        particles->update(dt);
    }

    // Update weather
    auto weather = ENGINE.getWeatherSystem();
    if (weather) {
        weather->update(dt);
    }
}

void RaidScene::render() {
    auto renderer = ENGINE.getRenderEngine();
    if (!renderer) return;

    // Setup camera
    Camera& cam = renderer->getCamera();
    cam.position = cameraPosition;
    cam.target = Vec3(
        cameraPosition.x + std::sinf(cameraRotation.y),
        cameraPosition.y + std::sinf(cameraRotation.x),
        cameraPosition.z + std::cosf(cameraRotation.y)
    );
    cam.updateMatrices();

    // Begin frame
    renderer->beginFrame();

    // Render all visible entities
    for (auto& pair : entities) {
        Entity& entity = pair.second;
        if (!entity.isVisible()) continue;

        // TODO: Load mesh from prefab and render
        // For now, just a placeholder
    }

    // Render particles
    auto particles = ENGINE.getParticleSystem();
    if (particles) {
        particles->render(cam);
    }

    // End frame
    renderer->endFrame();
}

void RaidScene::loadMap() {
    std::cout << "[RaidScene] Loading map..." << std::endl;

    // TODO: Load map geometry, navmesh, spawn points
    // For now, just create a ground plane

    Transform groundTransform;
    groundTransform.position = Vec3(0, -1, 0);
    groundTransform.scale = Vec3(100, 0.1f, 100);

    // Ground entity (not from prefab system)
    Entity ground;
    ground.id = nextEntityId++;
    ground.transform = groundTransform;
    ground.flags = ENTITY_ACTIVE | ENTITY_VISIBLE | ENTITY_PHYSICS;
    entities[ground.id] = ground;
}

void RaidScene::spawnPlayer() {
    std::cout << "[RaidScene] Spawning player..." << std::endl;

    Transform playerTransform;
    playerTransform.position = Vec3(0, 2, 0);  // Spawn above ground

    playerEntityId = spawnEntity(prefabs["player"], playerTransform);

    // Initialize camera
    cameraPosition = playerTransform.position + Vec3(0, 1.6f, 0);  // Eye level
    cameraRotation = Vec3(0, 0, 0);
}

void RaidScene::spawnAI() {
    std::cout << "[RaidScene] Spawning AI..." << std::endl;

    // Spawn 3 scavs
    for (int i = 0; i < 3; i++) {
        Transform scavTransform;
        scavTransform.position = Vec3(
            -10.0f + i * 10.0f,
            2.0f,
            20.0f
        );

        EntityId scavId = spawnEntity(prefabs["scav"], scavTransform);
        Entity* scav = getEntity(scavId);
        if (scav) {
            scav->flags |= ENTITY_AI;
        }
    }
}

void RaidScene::spawnLoot() {
    std::cout << "[RaidScene] Spawning loot..." << std::endl;

    // Spawn some loot crates
    for (int i = 0; i < 5; i++) {
        Transform crateTransform;
        crateTransform.position = Vec3(
            -15.0f + i * 7.5f,
            1.0f,
            10.0f
        );

        EntityId crateId = spawnEntity(prefabs["loot_crate"], crateTransform);
        Entity* crate = getEntity(crateId);
        if (crate) {
            crate->flags |= ENTITY_LOOTABLE;
        }
    }
}

EntityId RaidScene::spawnEntity(const Prefab& prefab, const Transform& transform) {
    EntityId eid = nextEntityId++;

    Entity entity;
    entity.id = eid;
    entity.transform = transform;
    entity.prefabId = prefab.id;
    entity.flags = ENTITY_ACTIVE | ENTITY_VISIBLE;

    if (prefab.hasPhysics) {
        entity.flags |= ENTITY_PHYSICS;
    }

    entities[eid] = entity;

    std::cout << "[RaidScene] Spawned entity: " << prefab.displayName << " (ID: " << eid << ")" << std::endl;

    return eid;
}

void RaidScene::destroyEntity(EntityId eid) {
    auto it = entities.find(eid);
    if (it != entities.end()) {
        std::cout << "[RaidScene] Destroyed entity ID: " << eid << std::endl;
        entities.erase(it);
    }
}

Entity* RaidScene::getEntity(EntityId eid) {
    auto it = entities.find(eid);
    return (it != entities.end()) ? &it->second : nullptr;
}

void RaidScene::processNetworkPackets() {
    while (networkClient->hasPackets()) {
        NetworkClient::ReceivedPacket packet = networkClient->getNextPacket();

        // Handle gameplay packets
        switch (packet.type) {
            case PacketType::PLAYER_MOVE:
                // TODO: Apply other players' movement
                break;

            case PacketType::PLAYER_SHOOT:
                // TODO: Spawn muzzle flash, apply damage
                break;

            case PacketType::PLAYER_DAMAGE:
                // TODO: Apply damage to entity
                break;

            case PacketType::PLAYER_DEATH:
                // TODO: Ragdoll, death animation
                break;

            case PacketType::EXTRACTION_COMPLETE:
                std::cout << "[RaidScene] Extraction complete! Returning to menu..." << std::endl;
                ENGINE.getSceneManager()->switchTo("menu");
                break;

            default:
                break;
        }
    }
}

void RaidScene::updatePlayer(float dt) {
    Entity* player = getEntity(playerEntityId);
    if (!player) return;

    // Simple WASD movement
    float speed = 5.0f;
    Vec3 movement(0, 0, 0);

    if (moveForward) movement.z += speed * dt;
    if (moveBackward) movement.z -= speed * dt;
    if (moveLeft) movement.x -= speed * dt;
    if (moveRight) movement.x += speed * dt;

    // Apply rotation
    float yaw = cameraRotation.y;
    Vec3 rotatedMovement(
        movement.x * std::cosf(yaw) - movement.z * std::sinf(yaw),
        0,
        movement.x * std::sinf(yaw) + movement.z * std::cosf(yaw)
    );

    player->transform.position = player->transform.position + rotatedMovement;

    // Update camera to follow player
    cameraPosition = player->transform.position + Vec3(0, 1.6f, 0);
}

void RaidScene::updateAI(float dt) {
    // Simple AI: wander towards player
    Entity* player = getEntity(playerEntityId);
    if (!player) return;

    for (auto& pair : entities) {
        Entity& entity = pair.second;
        if (!(entity.flags & ENTITY_AI)) continue;
        if (!entity.isActive()) continue;

        // Move towards player slowly
        Vec3 toPlayer = player->transform.position - entity.transform.position;
        float distance = toPlayer.length();

        if (distance > 2.0f) {  // Don't get too close
            Vec3 direction = toPlayer.normalized();
            entity.transform.position = entity.transform.position + direction * (1.0f * dt);
        }
    }
}

void RaidScene::updatePhysics(float dt) {
    auto physics = ENGINE.getPhysicsEngine();
    if (!physics) return;

    // TODO: Sync entity transforms with physics bodies
    // For now, entities just use their transforms directly
}

void RaidScene::handleInput(char key) {
    switch (key) {
        case 'w': case 'W': moveForward = true; break;
        case 's': case 'S': moveBackward = true; break;
        case 'a': case 'A': moveLeft = true; break;
        case 'd': case 'D': moveRight = true; break;
        case 'e': case 'E':
            std::cout << "[RaidScene] Extract key pressed" << std::endl;
            // TODO: Check if near extraction point
            break;
    }
}

void RaidScene::handleMouseClick(float x, float y) {
    // TODO: Raycast and shoot
    std::cout << "[RaidScene] Mouse click at " << x << ", " << y << std::endl;
}

void RaidScene::handleMouseMove(float x, float y) {
    // Update camera rotation based on mouse movement
    float sensitivity = 0.001f;
    cameraRotation.y += (x - mouseX) * sensitivity;  // Yaw
    cameraRotation.x += (y - mouseY) * sensitivity;  // Pitch

    // Clamp pitch
    if (cameraRotation.x > 1.5f) cameraRotation.x = 1.5f;
    if (cameraRotation.x < -1.5f) cameraRotation.x = -1.5f;

    mouseX = x;
    mouseY = y;
}
