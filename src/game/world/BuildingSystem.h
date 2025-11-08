#pragma once

#include "../../common/DataStructures.h"
#include <vector>
#include <memory>
#include <string>

/**
 * Room - Interior room within a building
 */
struct Room {
    enum class RoomType {
        GENERIC,
        BEDROOM,
        BATHROOM,
        KITCHEN,
        LIVING_ROOM,
        OFFICE,
        STORAGE,
        HALLWAY,
        STAIRWELL
    };

    RoomType type;
    Vec3 position;       // Position within building
    Vec3 size;           // Width, height, depth
    int floorNumber;
    std::vector<Vec3> lootSpawnPoints;  // Positions where loot can spawn
    std::vector<Vec3> coverPoints;      // Tactical cover positions

    Room() : type(RoomType::GENERIC), floorNumber(0) {}
};

/**
 * Door - Entrance/exit point
 */
struct Door {
    Vec3 position;
    Vec3 rotation;
    bool isExterior;  // Leads outside
    bool locked;
    int requiredKey;  // 0 = no key needed

    Door() : isExterior(false), locked(false), requiredKey(0) {}
};

/**
 * Window - Can be used for entry/sightlines
 */
struct Window {
    Vec3 position;
    Vec3 rotation;
    bool breakable;
    bool broken;

    Window() : breakable(true), broken(false) {}
};

/**
 * Building - Complete building with interior
 */
class Building {
public:
    enum class BuildingType {
        SMALL_HOUSE,      // 1-2 rooms, 1 floor
        MEDIUM_HOUSE,     // 3-5 rooms, 1-2 floors
        LARGE_HOUSE,      // 6+ rooms, 2-3 floors
        APARTMENT,        // Multiple units
        WAREHOUSE,        // Large open space
        OFFICE_BUILDING,  // Multiple floors, offices
        STORE,            // Shop with storage
        FACTORY           // Industrial
    };

    Building(BuildingType type);
    ~Building() = default;

    // Generation
    void generate(const Vec3& worldPosition);
    void generateRooms();
    void generateDoors();
    void generateWindows();
    void generateLootPoints();

    // Queries
    Room* getRoomAt(const Vec3& position);
    std::vector<Room*> getRoomsByType(Room::RoomType type);
    std::vector<Vec3> getAllLootSpawnPoints() const;
    std::vector<Vec3> getAllCoverPoints() const;

    // Doors and windows
    Door* getNearestDoor(const Vec3& position, float maxDistance = 5.0f);
    bool canEnterAt(const Vec3& position) const;

    // Getters
    BuildingType getType() const { return buildingType; }
    Vec3 getPosition() const { return position; }
    Vec3 getSize() const { return size; }
    int getNumFloors() const { return numFloors; }
    const std::vector<Room>& getRooms() const { return rooms; }
    const std::vector<Door>& getDoors() const { return doors; }
    const std::vector<Window>& getWindows() const { return windows; }

    // Mesh/rendering
    uint32_t getMeshId() const { return meshId; }
    uint32_t getInteriorMeshId() const { return interiorMeshId; }

private:
    BuildingType buildingType;
    Vec3 position;
    Vec3 size;
    Vec3 rotation;
    int numFloors;

    std::vector<Room> rooms;
    std::vector<Door> doors;
    std::vector<Window> windows;

    uint32_t meshId;
    uint32_t interiorMeshId;
    uint32_t collisionMeshId;

    // Generation helpers
    void generateSmallHouse();
    void generateMediumHouse();
    void generateLargeHouse();
    void generateWarehouse();
    void generateOfficeBuilding();

    void addRoom(Room::RoomType type, const Vec3& pos, const Vec3& size, int floor);
    void addDoor(const Vec3& pos, const Vec3& rot, bool exterior);
    void addWindow(const Vec3& pos, const Vec3& rot);
    void addLootPoint(Room& room, const Vec3& localPos);
};

/**
 * BuildingLayout - Template for building generation
 */
struct BuildingLayout {
    Building::BuildingType type;
    Vec3 minSize;
    Vec3 maxSize;
    int minFloors;
    int maxFloors;
    int minRooms;
    int maxRooms;
    float doorDensity;    // Doors per room
    float windowDensity;  // Windows per wall
    float lootDensity;    // Loot points per room

    BuildingLayout()
        : type(Building::BuildingType::SMALL_HOUSE),
          minSize(5, 3, 5), maxSize(10, 4, 10),
          minFloors(1), maxFloors(1),
          minRooms(2), maxRooms(4),
          doorDensity(1.5f), windowDensity(2.0f), lootDensity(3.0f) {}
};

/**
 * BuildingSystem - Manages all buildings in the world
 */
class BuildingSystem {
public:
    BuildingSystem();
    ~BuildingSystem();

    // Initialization
    void initialize();
    void shutdown();

    // Building creation
    Building* spawnBuilding(Building::BuildingType type, const Vec3& position);
    void removeBuilding(Building* building);

    // Procedural generation
    void generateBuildingsInArea(const Vec3& center, float radius, int numBuildings);
    void placeBuilding(Building::BuildingType type, const Vec3& position);

    // Queries
    Building* getBuildingAt(const Vec3& position);
    std::vector<Building*> getBuildingsInRadius(const Vec3& center, float radius);
    std::vector<Vec3> getAllLootSpawnPoints() const;

    // Update
    void update(float deltaTime);

    // Rendering
    void render();

    // Getters
    int getNumBuildings() const { return static_cast<int>(buildings.size()); }
    const std::vector<std::unique_ptr<Building>>& getBuildings() const { return buildings; }

    // Building layouts
    void registerLayout(const BuildingLayout& layout);
    const BuildingLayout* getLayout(Building::BuildingType type) const;

private:
    std::vector<std::unique_ptr<Building>> buildings;
    std::vector<BuildingLayout> layouts;

    void initializeDefaultLayouts();
    bool isValidBuildingPosition(const Vec3& position, const Vec3& size) const;
};
