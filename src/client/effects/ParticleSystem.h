// Particle System for Visual Effects
// Handles muzzle flashes, blood, bullet impacts, etc.

#pragma once

#include "raylib.h"
#include <vector>
#include <random>

struct Particle {
    Vector3 position;
    Vector3 velocity;
    Color color;
    float lifetime;
    float maxLifetime;
    float size;
    bool active;

    Particle()
        : position({0, 0, 0})
        , velocity({0, 0, 0})
        , color(WHITE)
        , lifetime(0)
        , maxLifetime(1.0f)
        , size(0.1f)
        , active(false)
    {
    }
};

enum class ParticleEffectType {
    MUZZLE_FLASH,
    BULLET_IMPACT,
    BLOOD_SPLATTER,
    SMOKE,
    DUST,
    EXTRACTION_GLOW,
    LOOT_SPARKLE
};

class ParticleSystem {
private:
    std::vector<Particle> particles;
    int maxParticles;
    int nextParticleIndex;

    std::random_device rd;
    std::mt19937 gen;

public:
    ParticleSystem(int maxParticles = 1000)
        : maxParticles(maxParticles)
        , nextParticleIndex(0)
        , gen(rd())
    {
        particles.resize(maxParticles);
    }

    // Emit a particle effect
    void emit(ParticleEffectType type, Vector3 position, Vector3 direction = {0, 1, 0}) {
        switch (type) {
            case ParticleEffectType::MUZZLE_FLASH:
                emitMuzzleFlash(position, direction);
                break;

            case ParticleEffectType::BULLET_IMPACT:
                emitBulletImpact(position, direction);
                break;

            case ParticleEffectType::BLOOD_SPLATTER:
                emitBloodSplatter(position);
                break;

            case ParticleEffectType::SMOKE:
                emitSmoke(position);
                break;

            case ParticleEffectType::DUST:
                emitDust(position);
                break;

            case ParticleEffectType::EXTRACTION_GLOW:
                emitExtractionGlow(position);
                break;

            case ParticleEffectType::LOOT_SPARKLE:
                emitLootSparkle(position);
                break;
        }
    }

    // Update all particles
    void update(float deltaTime) {
        for (Particle& p : particles) {
            if (!p.active) continue;

            // Update lifetime
            p.lifetime += deltaTime;
            if (p.lifetime >= p.maxLifetime) {
                p.active = false;
                continue;
            }

            // Apply velocity
            p.position.x += p.velocity.x * deltaTime;
            p.position.y += p.velocity.y * deltaTime;
            p.position.z += p.velocity.z * deltaTime;

            // Apply gravity
            p.velocity.y -= 9.8f * deltaTime;

            // Fade out color based on lifetime
            float alpha = 1.0f - (p.lifetime / p.maxLifetime);
            p.color.a = static_cast<unsigned char>(alpha * 255);

            // Reduce size over time
            p.size *= 0.98f;
        }
    }

    // Render all particles
    void render(Camera3D camera) {
        for (const Particle& p : particles) {
            if (!p.active) continue;

            // Draw particle as a billboard (always facing camera)
            DrawCube(p.position, p.size, p.size, p.size, p.color);
        }
    }

private:
    // Get next available particle
    Particle* getNextParticle() {
        // Simple ring buffer
        for (int i = 0; i < maxParticles; i++) {
            int index = (nextParticleIndex + i) % maxParticles;
            if (!particles[index].active) {
                nextParticleIndex = (index + 1) % maxParticles;
                return &particles[index];
            }
        }

        // If all particles active, overwrite the oldest
        Particle* p = &particles[nextParticleIndex];
        nextParticleIndex = (nextParticleIndex + 1) % maxParticles;
        return p;
    }

    // Random float between min and max
    float randomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen);
    }

    // Emit muzzle flash particles
    void emitMuzzleFlash(Vector3 position, Vector3 direction) {
        for (int i = 0; i < 5; i++) {
            Particle* p = getNextParticle();
            if (!p) return;

            p->position = position;
            p->velocity = {
                direction.x * randomFloat(2.0f, 5.0f) + randomFloat(-0.5f, 0.5f),
                direction.y * randomFloat(2.0f, 5.0f) + randomFloat(-0.5f, 0.5f),
                direction.z * randomFloat(2.0f, 5.0f) + randomFloat(-0.5f, 0.5f)
            };
            p->color = { 255, static_cast<unsigned char>(randomFloat(150, 255)), 0, 255 }; // Yellow-orange
            p->lifetime = 0;
            p->maxLifetime = 0.1f;
            p->size = randomFloat(0.1f, 0.3f);
            p->active = true;
        }
    }

    // Emit bullet impact particles
    void emitBulletImpact(Vector3 position, Vector3 normal) {
        for (int i = 0; i < 10; i++) {
            Particle* p = getNextParticle();
            if (!p) return;

            p->position = position;
            p->velocity = {
                normal.x * randomFloat(1.0f, 3.0f) + randomFloat(-1.0f, 1.0f),
                normal.y * randomFloat(1.0f, 3.0f) + randomFloat(0.5f, 2.0f),
                normal.z * randomFloat(1.0f, 3.0f) + randomFloat(-1.0f, 1.0f)
            };
            p->color = GRAY;
            p->lifetime = 0;
            p->maxLifetime = randomFloat(0.3f, 0.6f);
            p->size = randomFloat(0.05f, 0.15f);
            p->active = true;
        }
    }

    // Emit blood splatter particles
    void emitBloodSplatter(Vector3 position) {
        for (int i = 0; i < 15; i++) {
            Particle* p = getNextParticle();
            if (!p) return;

            p->position = position;
            p->velocity = {
                randomFloat(-2.0f, 2.0f),
                randomFloat(0.5f, 3.0f),
                randomFloat(-2.0f, 2.0f)
            };
            p->color = { 180, 0, 0, 255 }; // Dark red
            p->lifetime = 0;
            p->maxLifetime = randomFloat(0.5f, 1.0f);
            p->size = randomFloat(0.05f, 0.2f);
            p->active = true;
        }
    }

    // Emit smoke particles
    void emitSmoke(Vector3 position) {
        Particle* p = getNextParticle();
        if (!p) return;

        p->position = position;
        p->velocity = {
            randomFloat(-0.2f, 0.2f),
            randomFloat(0.5f, 1.5f),  // Smoke rises
            randomFloat(-0.2f, 0.2f)
        };
        p->color = { 100, 100, 100, 180 }; // Gray smoke
        p->lifetime = 0;
        p->maxLifetime = randomFloat(1.0f, 2.0f);
        p->size = randomFloat(0.3f, 0.6f);
        p->active = true;
    }

    // Emit dust particles
    void emitDust(Vector3 position) {
        for (int i = 0; i < 5; i++) {
            Particle* p = getNextParticle();
            if (!p) return;

            p->position = position;
            p->velocity = {
                randomFloat(-0.5f, 0.5f),
                randomFloat(0.1f, 0.5f),
                randomFloat(-0.5f, 0.5f)
            };
            p->color = { 200, 180, 150, 180 }; // Tan/brown dust
            p->lifetime = 0;
            p->maxLifetime = randomFloat(0.5f, 1.0f);
            p->size = randomFloat(0.1f, 0.3f);
            p->active = true;
        }
    }

    // Emit extraction zone glow particles
    void emitExtractionGlow(Vector3 position) {
        Particle* p = getNextParticle();
        if (!p) return;

        p->position = {
            position.x + randomFloat(-1.5f, 1.5f),
            position.y + randomFloat(0.0f, 0.5f),
            position.z + randomFloat(-1.5f, 1.5f)
        };
        p->velocity = {
            randomFloat(-0.1f, 0.1f),
            randomFloat(0.3f, 0.8f),  // Float upward
            randomFloat(-0.1f, 0.1f)
        };
        p->color = { 0, 255, 100, 200 }; // Green glow
        p->lifetime = 0;
        p->maxLifetime = randomFloat(1.5f, 2.5f);
        p->size = randomFloat(0.2f, 0.4f);
        p->active = true;
    }

    // Emit loot sparkle particles
    void emitLootSparkle(Vector3 position) {
        Particle* p = getNextParticle();
        if (!p) return;

        p->position = {
            position.x + randomFloat(-0.3f, 0.3f),
            position.y + randomFloat(0.0f, 0.5f),
            position.z + randomFloat(-0.3f, 0.3f)
        };
        p->velocity = {
            randomFloat(-0.2f, 0.2f),
            randomFloat(0.2f, 0.5f),
            randomFloat(-0.2f, 0.2f)
        };
        p->color = { 255, 215, 0, 255 }; // Gold sparkle
        p->lifetime = 0;
        p->maxLifetime = randomFloat(0.5f, 1.0f);
        p->size = randomFloat(0.05f, 0.15f);
        p->active = true;
    }
};

// Continuous particle emitter (for looping effects like extraction zones)
class ContinuousEmitter {
private:
    ParticleSystem& particleSystem;
    ParticleEffectType effectType;
    Vector3 position;
    float emitRate;  // Particles per second
    float accumulator;
    bool active;

public:
    ContinuousEmitter(ParticleSystem& ps, ParticleEffectType type, Vector3 pos, float rate)
        : particleSystem(ps)
        , effectType(type)
        , position(pos)
        , emitRate(rate)
        , accumulator(0)
        , active(true)
    {
    }

    void update(float deltaTime) {
        if (!active) return;

        accumulator += deltaTime;

        float emitInterval = 1.0f / emitRate;
        while (accumulator >= emitInterval) {
            particleSystem.emit(effectType, position);
            accumulator -= emitInterval;
        }
    }

    void setPosition(Vector3 pos) { position = pos; }
    void setActive(bool a) { active = a; }
    bool isActive() const { return active; }
};
