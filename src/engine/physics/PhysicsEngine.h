#pragma once

#include "../rendering/RenderEngine.h"
#include <vector>
#include <memory>

// Physics shapes
enum class ColliderType {
    SPHERE,
    BOX,
    CAPSULE,
    MESH,
    PLANE
};

// Collider base
struct Collider {
    ColliderType type;
    Vec3 center;
    bool isTrigger;
    int layer;

    Collider(ColliderType type) : type(type), isTrigger(false), layer(0) {}
    virtual ~Collider() = default;

    virtual bool checkCollision(const Collider* other, Vec3& contactPoint, Vec3& normal) const = 0;
    virtual Vec3 getSupport(const Vec3& direction) const = 0;
};

struct SphereCollider : public Collider {
    float radius;

    SphereCollider(float r = 1.0f) : Collider(ColliderType::SPHERE), radius(r) {}

    bool checkCollision(const Collider* other, Vec3& contactPoint, Vec3& normal) const override;
    Vec3 getSupport(const Vec3& direction) const override;
};

struct BoxCollider : public Collider {
    Vec3 size;  // Half extents

    BoxCollider(const Vec3& size = Vec3(1, 1, 1)) : Collider(ColliderType::BOX), size(size) {}

    bool checkCollision(const Collider* other, Vec3& contactPoint, Vec3& normal) const override;
    Vec3 getSupport(const Vec3& direction) const override;
};

struct CapsuleCollider : public Collider {
    float radius;
    float height;

    CapsuleCollider(float r = 0.5f, float h = 2.0f)
        : Collider(ColliderType::CAPSULE), radius(r), height(h) {}

    bool checkCollision(const Collider* other, Vec3& contactPoint, Vec3& normal) const override;
    Vec3 getSupport(const Vec3& direction) const override;
};

struct PlaneCollider : public Collider {
    Vec3 normal;
    float distance;

    PlaneCollider(const Vec3& n = Vec3(0, 1, 0), float d = 0.0f)
        : Collider(ColliderType::PLANE), normal(n), distance(d) {}

    bool checkCollision(const Collider* other, Vec3& contactPoint, Vec3& normal) const override;
    Vec3 getSupport(const Vec3& direction) const override;
};

// Rigidbody
struct Rigidbody {
    // Transform
    Vec3 position;
    Vec3 rotation;  // Euler angles
    Vec3 scale;

    // Physics properties
    float mass;
    float drag;
    float angularDrag;
    bool useGravity;
    bool isKinematic;

    // Motion
    Vec3 velocity;
    Vec3 angularVelocity;
    Vec3 acceleration;

    // Forces
    Vec3 force;
    Vec3 torque;

    // Collider
    std::unique_ptr<Collider> collider;

    // Constraints
    bool freezePositionX;
    bool freezePositionY;
    bool freezePositionZ;
    bool freezeRotationX;
    bool freezeRotationY;
    bool freezeRotationZ;

    // Material
    float bounciness;
    float friction;

    Rigidbody() : mass(1.0f), drag(0.0f), angularDrag(0.05f),
                  useGravity(true), isKinematic(false),
                  freezePositionX(false), freezePositionY(false), freezePositionZ(false),
                  freezeRotationX(false), freezeRotationY(false), freezeRotationZ(false),
                  bounciness(0.3f), friction(0.5f), scale(1, 1, 1) {}

    void applyForce(const Vec3& f);
    void applyImpulse(const Vec3& impulse);
    void applyTorque(const Vec3& t);
    void addVelocity(const Vec3& v);
};

// Collision info
struct CollisionInfo {
    Rigidbody* bodyA;
    Rigidbody* bodyB;
    Vec3 contactPoint;
    Vec3 normal;
    float penetration;
    float relativeVelocity;
};

// Raycast hit
struct RaycastHit {
    Rigidbody* body;
    Vec3 point;
    Vec3 normal;
    float distance;
    bool hit;

    RaycastHit() : body(nullptr), distance(0.0f), hit(false) {}
};

// Physics callbacks
class IPhysicsCallback {
public:
    virtual ~IPhysicsCallback() = default;
    virtual void onCollisionEnter(const CollisionInfo& collision) {}
    virtual void onCollisionStay(const CollisionInfo& collision) {}
    virtual void onCollisionExit(Rigidbody* other) {}
    virtual void onTriggerEnter(Rigidbody* other) {}
    virtual void onTriggerExit(Rigidbody* other) {}
};

// Physics layers
enum PhysicsLayer {
    LAYER_DEFAULT = 0,
    LAYER_PLAYER = 1,
    LAYER_ENEMY = 2,
    LAYER_PROJECTILE = 3,
    LAYER_TERRAIN = 4,
    LAYER_TRIGGER = 5,
    LAYER_RAGDOLL = 6,
    LAYER_IGNORE_RAYCAST = 7
};

// Physics engine
class PhysicsEngine {
public:
    PhysicsEngine();
    ~PhysicsEngine();

    bool initialize(const Vec3& gravity = Vec3(0, -9.81f, 0));
    void shutdown();

    // Simulation
    void step(float deltaTime);
    void fixedUpdate(float fixedTimeStep);

    // Rigidbody management
    Rigidbody* createRigidbody();
    void removeRigidbody(Rigidbody* body);
    void clearRigidbodies();

    // Raycasting
    bool raycast(const Vec3& origin, const Vec3& direction, float maxDistance, RaycastHit& hit);
    std::vector<RaycastHit> raycastAll(const Vec3& origin, const Vec3& direction, float maxDistance);
    bool sphereCast(const Vec3& origin, float radius, const Vec3& direction, float maxDistance, RaycastHit& hit);

    // Overlap queries
    std::vector<Rigidbody*> overlapSphere(const Vec3& center, float radius);
    std::vector<Rigidbody*> overlapBox(const Vec3& center, const Vec3& halfExtents);

    // Settings
    void setGravity(const Vec3& g) { gravity = g; }
    Vec3 getGravity() const { return gravity; }
    void setMaxVelocity(float max) { maxVelocity = max; }
    void setIterations(int iterations) { this->iterations = iterations; }
    void setLayerCollisionMatrix(int layer1, int layer2, bool collide);
    bool getLayerCollisionMatrix(int layer1, int layer2) const;

    // Debug
    void debugDraw();
    void setDebugDraw(bool enabled) { debugDrawEnabled = enabled; }

    // Callbacks
    void setCallback(IPhysicsCallback* callback) { this->callback = callback; }

    // Stats
    int getActiveRigidbodies() const { return (int)rigidbodies.size(); }
    int getCollisionChecks() const { return collisionChecks; }
    void resetStats();

private:
    // Integration
    void integrate(Rigidbody* body, float dt);
    void applyGravity(Rigidbody* body);
    void applyDrag(Rigidbody* body, float dt);

    // Collision detection
    void detectCollisions();
    bool checkCollision(Rigidbody* a, Rigidbody* b, CollisionInfo& info);
    void resolveCollision(const CollisionInfo& collision);

    // Narrow phase
    bool sphereVsSphere(const SphereCollider* a, const SphereCollider* b,
                       const Vec3& posA, const Vec3& posB,
                       Vec3& contactPoint, Vec3& normal, float& penetration);
    bool boxVsBox(const BoxCollider* a, const BoxCollider* b,
                 const Vec3& posA, const Vec3& posB,
                 Vec3& contactPoint, Vec3& normal, float& penetration);
    bool sphereVsBox(const SphereCollider* sphere, const BoxCollider* box,
                    const Vec3& spherePos, const Vec3& boxPos,
                    Vec3& contactPoint, Vec3& normal, float& penetration);
    bool capsuleVsBox(const CapsuleCollider* capsule, const BoxCollider* box,
                     const Vec3& capsulePos, const Vec3& boxPos,
                     Vec3& contactPoint, Vec3& normal, float& penetration);

    // Spatial partitioning
    struct OctreeNode {
        Vec3 center;
        Vec3 halfSize;
        std::vector<Rigidbody*> bodies;
        std::unique_ptr<OctreeNode> children[8];
        bool isLeaf;

        OctreeNode(const Vec3& c, const Vec3& hs) : center(c), halfSize(hs), isLeaf(true) {}
    };
    std::unique_ptr<OctreeNode> octree;
    void buildOctree();
    void insertIntoOctree(Rigidbody* body, OctreeNode* node);
    void queryOctree(const Vec3& center, float radius, std::vector<Rigidbody*>& results, OctreeNode* node);

    // Data
    std::vector<std::unique_ptr<Rigidbody>> rigidbodies;
    std::vector<CollisionInfo> collisions;
    Vec3 gravity;
    float maxVelocity;
    int iterations;
    bool layerCollisionMatrix[32][32];

    // Debug
    bool debugDrawEnabled;

    // Callback
    IPhysicsCallback* callback;

    // Stats
    int collisionChecks;
    int collisionResolutions;
};
