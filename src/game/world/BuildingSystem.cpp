#include "BuildingSystem.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <iostream>

// ========== Building Implementation ==========

Building::Building(BuildingType type)
    : buildingType(type), numFloors(1), meshId(0), interiorMeshId(0), collisionMeshId(0) {
}

void Building::generate(const Vec3& worldPosition) {
    position = worldPosition;

    // Generate based on building type
    switch (buildingType) {
        case BuildingType::SMALL_HOUSE:
            generateSmallHouse();
            break;
        case BuildingType::MEDIUM_HOUSE:
            generateMediumHouse();
            break;
        case BuildingType::LARGE_HOUSE:
            generateLargeHouse();
            break;
        case BuildingType::WAREHOUSE:
            generateWarehouse();
            break;
        case BuildingType::OFFICE_BUILDING:
            generateOfficeBuilding();
            break;
        default:
            generateSmallHouse();
            break;
    }

    generateDoors();
    generateWindows();
    generateLootPoints();

    std::cout << "[Building] Generated " << static_cast<int>(buildingType)
              << " with " << rooms.size() << " rooms" << std::endl;
}

void Building::generateSmallHouse() {
    size = Vec3(8, 3, 8);
    numFloors = 1;

    // Create 2-3 rooms
    addRoom(Room::RoomType::LIVING_ROOM, Vec3(0, 0, 0), Vec3(5, 3, 4), 0);
    addRoom(Room::RoomType::BEDROOM, Vec3(5, 0, 0), Vec3(3, 3, 4), 0);
    addRoom(Room::RoomType::BATHROOM, Vec3(5, 0, 4), Vec3(3, 3, 4), 0);
}

void Building::generateMediumHouse() {
    size = Vec3(12, 6, 10);
    numFloors = 2;

    // First floor
    addRoom(Room::RoomType::LIVING_ROOM, Vec3(0, 0, 0), Vec3(6, 3, 5), 0);
    addRoom(Room::RoomType::KITCHEN, Vec3(6, 0, 0), Vec3(6, 3, 5), 0);
    addRoom(Room::RoomType::BATHROOM, Vec3(0, 0, 5), Vec3(4, 3, 5), 0);
    addRoom(Room::RoomType::STAIRWELL, Vec3(4, 0, 5), Vec3(2, 6, 2), 0);

    // Second floor
    addRoom(Room::RoomType::BEDROOM, Vec3(0, 3, 0), Vec3(5, 3, 4), 1);
    addRoom(Room::RoomType::BEDROOM, Vec3(0, 3, 4), Vec3(5, 3, 4), 1);
    addRoom(Room::RoomType::BEDROOM, Vec3(6, 3, 0), Vec3(6, 3, 5), 1);
    addRoom(Room::RoomType::BATHROOM, Vec3(6, 3, 5), Vec3(6, 3, 3), 1);
}

void Building::generateLargeHouse() {
    size = Vec3(16, 9, 14);
    numFloors = 3;

    // First floor - Living areas
    addRoom(Room::RoomType::LIVING_ROOM, Vec3(0, 0, 0), Vec3(8, 3, 7), 0);
    addRoom(Room::RoomType::KITCHEN, Vec3(8, 0, 0), Vec3(8, 3, 7), 0);
    addRoom(Room::RoomType::OFFICE, Vec3(0, 0, 7), Vec3(6, 3, 7), 0);
    addRoom(Room::RoomType::BATHROOM, Vec3(6, 0, 7), Vec3(4, 3, 4), 0);
    addRoom(Room::RoomType::STAIRWELL, Vec3(10, 0, 7), Vec3(3, 9, 3), 0);

    // Second floor - Bedrooms
    addRoom(Room::RoomType::BEDROOM, Vec3(0, 3, 0), Vec3(6, 3, 6), 1);
    addRoom(Room::RoomType::BEDROOM, Vec3(6, 3, 0), Vec3(6, 3, 6), 1);
    addRoom(Room::RoomType::BEDROOM, Vec3(0, 3, 6), Vec3(5, 3, 5), 1);
    addRoom(Room::RoomType::BATHROOM, Vec3(5, 3, 6), Vec3(4, 3, 4), 1);
    addRoom(Room::RoomType::BEDROOM, Vec3(13, 3, 0), Vec3(3, 3, 6), 1);

    // Third floor - Attic/Storage
    addRoom(Room::RoomType::STORAGE, Vec3(0, 6, 0), Vec3(10, 3, 10), 2);
}

void Building::generateWarehouse() {
    size = Vec3(30, 8, 20);
    numFloors = 1;

    // Large open space
    addRoom(Room::RoomType::GENERIC, Vec3(0, 0, 0), Vec3(25, 8, 20), 0);

    // Office area
    addRoom(Room::RoomType::OFFICE, Vec3(25, 0, 0), Vec3(5, 4, 10), 0);

    // Storage rooms
    addRoom(Room::RoomType::STORAGE, Vec3(25, 0, 10), Vec3(5, 4, 5), 0);
    addRoom(Room::RoomType::STORAGE, Vec3(25, 0, 15), Vec3(5, 4, 5), 0);
}

void Building::generateOfficeBuilding() {
    size = Vec3(20, 12, 15);
    numFloors = 4;

    for (int floor = 0; floor < numFloors; floor++) {
        float y = floor * 3.0f;

        // Offices
        addRoom(Room::RoomType::OFFICE, Vec3(0, y, 0), Vec3(6, 3, 5), floor);
        addRoom(Room::RoomType::OFFICE, Vec3(6, y, 0), Vec3(6, 3, 5), floor);
        addRoom(Room::RoomType::OFFICE, Vec3(12, y, 0), Vec3(6, 3, 5), floor);

        addRoom(Room::RoomType::OFFICE, Vec3(0, y, 5), Vec3(6, 3, 5), floor);
        addRoom(Room::RoomType::OFFICE, Vec3(6, y, 5), Vec3(6, 3, 5), floor);
        addRoom(Room::RoomType::OFFICE, Vec3(12, y, 5), Vec3(6, 3, 5), floor);

        // Hallway
        addRoom(Room::RoomType::HALLWAY, Vec3(0, y, 10), Vec3(15, 3, 3), floor);

        // Stairwell
        addRoom(Room::RoomType::STAIRWELL, Vec3(15, y, 0), Vec3(5, 3, 5), floor);

        // Bathroom
        addRoom(Room::RoomType::BATHROOM, Vec3(15, y, 10), Vec3(5, 3, 5), floor);
    }
}

void Building::generateRooms() {
    // Rooms are generated in the type-specific generation functions
}

void Building::generateDoors() {
    // Add exterior entrance
    addDoor(Vec3(size.x * 0.5f, 0, 0), Vec3(0, 0, 0), true);

    // Add interior doors between rooms
    for (size_t i = 0; i < rooms.size(); i++) {
        for (size_t j = i + 1; j < rooms.size(); j++) {
            Room& room1 = rooms[i];
            Room& room2 = rooms[j];

            // Check if rooms are adjacent and on same floor
            if (room1.floorNumber != room2.floorNumber) continue;

            // Check if rooms share a wall
            bool adjacent = false;
            Vec3 doorPos;

            // Check X adjacency
            if (std::abs((room1.position.x + room1.size.x) - room2.position.x) < 0.5f) {
                adjacent = true;
                doorPos = Vec3(room2.position.x, room1.position.y,
                              (room1.position.z + room2.position.z) * 0.5f);
            }
            else if (std::abs((room2.position.x + room2.size.x) - room1.position.x) < 0.5f) {
                adjacent = true;
                doorPos = Vec3(room1.position.x, room1.position.y,
                              (room1.position.z + room2.position.z) * 0.5f);
            }

            // Check Z adjacency
            if (std::abs((room1.position.z + room1.size.z) - room2.position.z) < 0.5f) {
                adjacent = true;
                doorPos = Vec3((room1.position.x + room2.position.x) * 0.5f,
                              room1.position.y, room2.position.z);
            }
            else if (std::abs((room2.position.z + room2.size.z) - room1.position.z) < 0.5f) {
                adjacent = true;
                doorPos = Vec3((room1.position.x + room2.position.x) * 0.5f,
                              room1.position.y, room1.position.z);
            }

            if (adjacent) {
                addDoor(doorPos, Vec3(0, 0, 0), false);
            }
        }
    }
}

void Building::generateWindows() {
    // Add windows on exterior walls
    int windowsPerWall = 2;

    // Front wall (Z = 0)
    for (int i = 0; i < windowsPerWall; i++) {
        float x = (i + 1) * size.x / (windowsPerWall + 1);
        addWindow(Vec3(x, 1.5f, 0), Vec3(0, 0, 0));
    }

    // Back wall (Z = size.z)
    for (int i = 0; i < windowsPerWall; i++) {
        float x = (i + 1) * size.x / (windowsPerWall + 1);
        addWindow(Vec3(x, 1.5f, size.z), Vec3(0, 180, 0));
    }

    // Left wall (X = 0)
    for (int i = 0; i < windowsPerWall; i++) {
        float z = (i + 1) * size.z / (windowsPerWall + 1);
        addWindow(Vec3(0, 1.5f, z), Vec3(0, -90, 0));
    }

    // Right wall (X = size.x)
    for (int i = 0; i < windowsPerWall; i++) {
        float z = (i + 1) * size.z / (windowsPerWall + 1);
        addWindow(Vec3(size.x, 1.5f, z), Vec3(0, 90, 0));
    }
}

void Building::generateLootPoints() {
    for (auto& room : rooms) {
        // Generate loot points based on room type
        int numPoints = 2;

        switch (room.type) {
            case Room::RoomType::OFFICE:
                numPoints = 4;
                break;
            case Room::RoomType::STORAGE:
                numPoints = 6;
                break;
            case Room::RoomType::BEDROOM:
                numPoints = 3;
                break;
            case Room::RoomType::KITCHEN:
                numPoints = 3;
                break;
            default:
                numPoints = 2;
                break;
        }

        // Distribute loot points around the room
        for (int i = 0; i < numPoints; i++) {
            Vec3 localPos;
            localPos.x = room.size.x * 0.2f + (i % 2) * room.size.x * 0.6f;
            localPos.y = 0.5f;  // On floor
            localPos.z = room.size.z * 0.2f + (i / 2) * room.size.z * 0.6f;

            addLootPoint(room, localPos);
        }
    }
}

Room* Building::getRoomAt(const Vec3& localPosition) {
    for (auto& room : rooms) {
        Vec3 relPos = localPosition;
        relPos.x -= room.position.x;
        relPos.y -= room.position.y;
        relPos.z -= room.position.z;

        if (relPos.x >= 0 && relPos.x <= room.size.x &&
            relPos.y >= 0 && relPos.y <= room.size.y &&
            relPos.z >= 0 && relPos.z <= room.size.z) {
            return &room;
        }
    }
    return nullptr;
}

std::vector<Room*> Building::getRoomsByType(Room::RoomType type) {
    std::vector<Room*> result;
    for (auto& room : rooms) {
        if (room.type == type) {
            result.push_back(&room);
        }
    }
    return result;
}

std::vector<Vec3> Building::getAllLootSpawnPoints() const {
    std::vector<Vec3> points;
    for (const auto& room : rooms) {
        for (const auto& lootPoint : room.lootSpawnPoints) {
            // Convert to world position
            Vec3 worldPos = position;
            worldPos.x += lootPoint.x;
            worldPos.y += lootPoint.y;
            worldPos.z += lootPoint.z;
            points.push_back(worldPos);
        }
    }
    return points;
}

std::vector<Vec3> Building::getAllCoverPoints() const {
    std::vector<Vec3> points;
    for (const auto& room : rooms) {
        for (const auto& coverPoint : room.coverPoints) {
            Vec3 worldPos = position;
            worldPos.x += coverPoint.x;
            worldPos.y += coverPoint.y;
            worldPos.z += coverPoint.z;
            points.push_back(worldPos);
        }
    }
    return points;
}

Door* Building::getNearestDoor(const Vec3& localPosition, float maxDistance) {
    Door* nearest = nullptr;
    float nearestDist = maxDistance * maxDistance;

    for (auto& door : doors) {
        float dx = door.position.x - localPosition.x;
        float dy = door.position.y - localPosition.y;
        float dz = door.position.z - localPosition.z;
        float distSq = dx * dx + dy * dy + dz * dz;

        if (distSq < nearestDist) {
            nearestDist = distSq;
            nearest = &door;
        }
    }

    return nearest;
}

bool Building::canEnterAt(const Vec3& localPosition) const {
    // Check if position is near an unlocked exterior door
    for (const auto& door : doors) {
        if (!door.isExterior || door.locked) continue;

        float dx = door.position.x - localPosition.x;
        float dz = door.position.z - localPosition.z;
        float distSq = dx * dx + dz * dz;

        if (distSq < 4.0f) {  // Within 2 meters
            return true;
        }
    }

    return false;
}

void Building::addRoom(Room::RoomType type, const Vec3& pos, const Vec3& sz, int floor) {
    Room room;
    room.type = type;
    room.position = pos;
    room.size = sz;
    room.floorNumber = floor;
    rooms.push_back(room);
}

void Building::addDoor(const Vec3& pos, const Vec3& rot, bool exterior) {
    Door door;
    door.position = pos;
    door.rotation = rot;
    door.isExterior = exterior;
    doors.push_back(door);
}

void Building::addWindow(const Vec3& pos, const Vec3& rot) {
    Window window;
    window.position = pos;
    window.rotation = rot;
    windows.push_back(window);
}

void Building::addLootPoint(Room& room, const Vec3& localPos) {
    Vec3 worldPos = room.position;
    worldPos.x += localPos.x;
    worldPos.y += localPos.y;
    worldPos.z += localPos.z;
    room.lootSpawnPoints.push_back(worldPos);
}

// ========== BuildingSystem Implementation ==========

BuildingSystem::BuildingSystem() {
}

BuildingSystem::~BuildingSystem() {
    shutdown();
}

void BuildingSystem::initialize() {
    initializeDefaultLayouts();
    std::cout << "[BuildingSystem] Initialized with " << layouts.size() << " layouts" << std::endl;
}

void BuildingSystem::shutdown() {
    buildings.clear();
    std::cout << "[BuildingSystem] Shutdown complete" << std::endl;
}

Building* BuildingSystem::spawnBuilding(Building::BuildingType type, const Vec3& position) {
    auto building = std::make_unique<Building>(type);
    building->generate(position);

    Building* ptr = building.get();
    buildings.push_back(std::move(building));

    std::cout << "[BuildingSystem] Spawned building at (" << position.x << ", "
              << position.y << ", " << position.z << ")" << std::endl;

    return ptr;
}

void BuildingSystem::removeBuilding(Building* building) {
    auto it = std::remove_if(buildings.begin(), buildings.end(),
        [building](const std::unique_ptr<Building>& b) {
            return b.get() == building;
        });

    if (it != buildings.end()) {
        buildings.erase(it, buildings.end());
    }
}

void BuildingSystem::generateBuildingsInArea(const Vec3& center, float radius, int numBuildings) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 6.28318f);  // 0 to 2*PI
    std::uniform_real_distribution<float> radiusDist(0.0f, radius);
    std::uniform_int_distribution<int> typeDist(0, 4);

    for (int i = 0; i < numBuildings; i++) {
        // Random position in circle
        float angle = angleDist(gen);
        float r = radiusDist(gen);

        Vec3 position;
        position.x = center.x + r * std::cos(angle);
        position.y = center.y;
        position.z = center.z + r * std::sin(angle);

        // Random building type
        Building::BuildingType type = static_cast<Building::BuildingType>(typeDist(gen));

        placeBuilding(type, position);
    }

    std::cout << "[BuildingSystem] Generated " << numBuildings << " buildings in area" << std::endl;
}

void BuildingSystem::placeBuilding(Building::BuildingType type, const Vec3& position) {
    // Check if position is valid
    // TODO: Check terrain height, collision with other buildings, etc.

    spawnBuilding(type, position);
}

Building* BuildingSystem::getBuildingAt(const Vec3& position) {
    for (auto& building : buildings) {
        Vec3 relPos = position;
        relPos.x -= building->getPosition().x;
        relPos.y -= building->getPosition().y;
        relPos.z -= building->getPosition().z;

        Vec3 size = building->getSize();

        if (relPos.x >= 0 && relPos.x <= size.x &&
            relPos.y >= 0 && relPos.y <= size.y &&
            relPos.z >= 0 && relPos.z <= size.z) {
            return building.get();
        }
    }

    return nullptr;
}

std::vector<Building*> BuildingSystem::getBuildingsInRadius(const Vec3& center, float radius) {
    std::vector<Building*> result;
    float radiusSq = radius * radius;

    for (auto& building : buildings) {
        Vec3 buildingPos = building->getPosition();
        float dx = buildingPos.x - center.x;
        float dz = buildingPos.z - center.z;
        float distSq = dx * dx + dz * dz;

        if (distSq <= radiusSq) {
            result.push_back(building.get());
        }
    }

    return result;
}

std::vector<Vec3> BuildingSystem::getAllLootSpawnPoints() const {
    std::vector<Vec3> points;

    for (const auto& building : buildings) {
        auto buildingPoints = building->getAllLootSpawnPoints();
        points.insert(points.end(), buildingPoints.begin(), buildingPoints.end());
    }

    return points;
}

void BuildingSystem::update(float deltaTime) {
    // Update building state (doors opening/closing, etc.)
}

void BuildingSystem::render() {
    // Render all buildings
    // Would use RenderEngine in real implementation
}

void BuildingSystem::registerLayout(const BuildingLayout& layout) {
    layouts.push_back(layout);
}

const BuildingLayout* BuildingSystem::getLayout(Building::BuildingType type) const {
    for (const auto& layout : layouts) {
        if (layout.type == type) {
            return &layout;
        }
    }
    return nullptr;
}

bool BuildingSystem::isValidBuildingPosition(const Vec3& position, const Vec3& size) const {
    // Check collision with existing buildings
    for (const auto& building : buildings) {
        Vec3 bPos = building->getPosition();
        Vec3 bSize = building->getSize();

        // Simple AABB collision check
        if (position.x < bPos.x + bSize.x && position.x + size.x > bPos.x &&
            position.z < bPos.z + bSize.z && position.z + size.z > bPos.z) {
            return false;
        }
    }

    return true;
}

void BuildingSystem::initializeDefaultLayouts() {
    // Small House
    BuildingLayout smallHouse;
    smallHouse.type = Building::BuildingType::SMALL_HOUSE;
    smallHouse.minSize = Vec3(6, 3, 6);
    smallHouse.maxSize = Vec3(10, 4, 10);
    smallHouse.minFloors = 1;
    smallHouse.maxFloors = 1;
    smallHouse.minRooms = 2;
    smallHouse.maxRooms = 4;
    registerLayout(smallHouse);

    // Medium House
    BuildingLayout mediumHouse;
    mediumHouse.type = Building::BuildingType::MEDIUM_HOUSE;
    mediumHouse.minSize = Vec3(10, 6, 8);
    mediumHouse.maxSize = Vec3(15, 8, 12);
    mediumHouse.minFloors = 1;
    mediumHouse.maxFloors = 2;
    mediumHouse.minRooms = 4;
    mediumHouse.maxRooms = 8;
    registerLayout(mediumHouse);

    // Warehouse
    BuildingLayout warehouse;
    warehouse.type = Building::BuildingType::WAREHOUSE;
    warehouse.minSize = Vec3(20, 6, 15);
    warehouse.maxSize = Vec3(40, 10, 30);
    warehouse.minFloors = 1;
    warehouse.maxFloors = 1;
    warehouse.minRooms = 1;
    warehouse.maxRooms = 5;
    warehouse.lootDensity = 8.0f;
    registerLayout(warehouse);
}
