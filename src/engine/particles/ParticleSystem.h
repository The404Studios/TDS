#pragma once

#include "../rendering/RenderEngine.h"
#include <vector>
#include <memory>
#include <functional>

// Particle structure
struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;

    Color color;
    Color startColor;
    Color endColor;

    float size;
    float startSize;
    float endSize;

    float rotation;
    float angularVelocity;

    float lifetime;
    float age;

    float mass;
    float drag;

    bool alive;

    Particle() : size(1.0f), startSize(1.0f), endSize(0.0f),
                 rotation(0.0f), angularVelocity(0.0f),
                 lifetime(1.0f), age(0.0f), mass(1.0f), drag(0.0f),
                 alive(true) {}
};

// Emitter shape
enum class EmitterShape {
    POINT,
    SPHERE,
    HEMISPHERE,
    BOX,
    CONE,
    CIRCLE,
    MESH
};

// Particle emitter configuration
struct EmitterConfig {
    // Emission
    float emissionRate;           // Particles per second
    int maxParticles;            // Maximum active particles
    bool looping;                 // Loop emission
    float duration;               // Emitter duration (-1 = infinite)

    // Shape
    EmitterShape shape;
    Vec3 shapeSize;              // For box/sphere
    float coneAngle;             // For cone

    // Initial particle properties
    Vec3 startVelocity;
    Vec3 startVelocityRandomness;

    Color startColor;
    Color startColorRandomness;

    Color endColor;
    Color endColorRandomness;

    float startSize;
    float startSizeRandomness;

    float endSize;
    float endSizeRandomness;

    float startRotation;
    float startRotationRandomness;

    float lifetime;
    float lifetimeRandomness;

    // Forces
    Vec3 gravity;
    Vec3 constantForce;
    float drag;

    // Angular velocity
    float angularVelocity;
    float angularVelocityRandomness;

    // Rendering
    GLuint texture;
    bool additive;                // Additive blending
    bool softParticles;           // Soft particles with depth buffer
    int sortMode;                 // 0=none, 1=distance, 2=age

    EmitterConfig() : emissionRate(10.0f), maxParticles(100), looping(true),
                     duration(-1.0f), shape(EmitterShape::POINT),
                     shapeSize(1, 1, 1), coneAngle(25.0f),
                     startSize(1.0f), startSizeRandomness(0.0f),
                     endSize(0.0f), endSizeRandomness(0.0f),
                     startRotation(0.0f), startRotationRandomness(0.0f),
                     lifetime(1.0f), lifetimeRandomness(0.2f),
                     drag(0.0f), angularVelocity(0.0f), angularVelocityRandomness(0.0f),
                     texture(0), additive(false), softParticles(false), sortMode(1) {}
};

// Particle emitter
class ParticleEmitter {
public:
    ParticleEmitter(const EmitterConfig& config = EmitterConfig());
    ~ParticleEmitter();

    // Lifecycle
    void update(float deltaTime);
    void render(const Camera& camera);

    // Control
    void play();
    void pause();
    void stop();
    void restart();
    bool isPlaying() const { return playing; }
    bool isPaused() const { return paused; }

    // Configuration
    void setConfig(const EmitterConfig& cfg) { config = cfg; }
    const EmitterConfig& getConfig() const { return config; }

    // Transform
    void setPosition(const Vec3& pos) { position = pos; }
    Vec3 getPosition() const { return position; }
    void setRotation(const Vec3& rot) { rotation = rot; }
    Vec3 getRotation() const { return rotation; }

    // Manual emission
    void emit(int count);
    void burst(int count);

    // Particle access
    int getActiveParticleCount() const;
    const std::vector<Particle>& getParticles() const { return particles; }

    // Custom update function
    using ParticleUpdateFunc = std::function<void(Particle&, float)>;
    void setCustomUpdate(ParticleUpdateFunc func) { customUpdate = func; }

private:
    void emitParticle();
    void updateParticle(Particle& p, float dt);
    Vec3 getRandomPointInShape();
    float randomRange(float min, float max);

    EmitterConfig config;
    std::vector<Particle> particles;

    Vec3 position;
    Vec3 rotation;

    bool playing;
    bool paused;
    float emissionTimer;
    float ageTimer;

    ParticleUpdateFunc customUpdate;
};

// Particle system presets
namespace ParticlePresets {
    EmitterConfig createFire();
    EmitterConfig createSmoke();
    EmitterConfig createExplosion();
    EmitterConfig createSparks();
    EmitterConfig createRain();
    EmitterConfig createSnow();
    EmitterConfig createMuzzleFlash();
    EmitterConfig createBlood();
    EmitterConfig createDust();
    EmitterConfig createMagic();
    EmitterConfig createTrail();
}

// Particle system manager
class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    bool initialize(int maxParticles = 10000);
    void shutdown();

    // Emitter management
    ParticleEmitter* createEmitter(const EmitterConfig& config = EmitterConfig());
    void removeEmitter(ParticleEmitter* emitter);
    void clearEmitters();

    // Update and render
    void update(float deltaTime);
    void render(const Camera& camera);

    // Quick effects (one-shot)
    void playEffect(const EmitterConfig& config, const Vec3& position, const Vec3& rotation = Vec3());

    // Presets
    void playFire(const Vec3& position);
    void playSmoke(const Vec3& position);
    void playExplosion(const Vec3& position);
    void playSparks(const Vec3& position);
    void playMuzzleFlash(const Vec3& position, const Vec3& direction);
    void playBlood(const Vec3& position);
    void playDust(const Vec3& position);

    // Settings
    void setGlobalGravity(const Vec3& gravity) { globalGravity = gravity; }
    void setMaxParticles(int max) { maxParticles = max; }
    void setCullingDistance(float distance) { cullingDistance = distance; }
    void setEnableCulling(bool enabled) { enableCulling = enabled; }

    // Debug
    void setDebugDraw(bool enabled) { debugDraw = enabled; }

    // Stats
    int getActiveEmitters() const { return (int)emitters.size(); }
    int getTotalActiveParticles() const;

private:
    std::vector<std::unique_ptr<ParticleEmitter>> emitters;
    Vec3 globalGravity;
    int maxParticles;
    float cullingDistance;
    bool enableCulling;
    bool debugDraw;

    // Rendering
    GLuint particleVAO;
    GLuint particleVBO;
    void initializeRendering();
    void cleanupRendering();
};
