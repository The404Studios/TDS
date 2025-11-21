// Animation Controller for raylib models
// Handles character animations (idle, walk, run, shoot, die, etc.)

#pragma once

#include "../RaylibPlatform.h"  // Platform-aware raylib header
#include <string>
#include <unordered_map>
#include <vector>

enum class AnimationType {
    IDLE,
    WALK,
    RUN,
    CROUCH,
    JUMP,
    SHOOT,
    RELOAD,
    MELEE,
    HIT,
    DIE,
    LOOT
};

struct AnimationClip {
    std::string name;
    int startFrame;
    int endFrame;
    float speed;
    bool loop;

    AnimationClip() : startFrame(0), endFrame(0), speed(1.0f), loop(true) {}

    AnimationClip(const std::string& n, int start, int end, float spd = 1.0f, bool lp = true)
        : name(n), startFrame(start), endFrame(end), speed(spd), loop(lp) {}
};

class AnimationController {
private:
    Model m_model;
    ModelAnimation* m_animations;
    int m_animationCount;

    std::unordered_map<AnimationType, AnimationClip> m_clips;

    AnimationType m_currentAnimation;
    int m_currentFrame;
    float m_frameAccumulator;
    bool m_isPlaying;
    bool m_modelLoaded;

public:
    AnimationController()
        : m_animations(nullptr)
        , m_animationCount(0)
        , m_currentAnimation(AnimationType::IDLE)
        , m_currentFrame(0)
        , m_frameAccumulator(0.0f)
        , m_isPlaying(true)
        , m_modelLoaded(false)
    {
    }

    ~AnimationController() {
        unload();
    }

    // Load model with animations
    bool loadModel(const std::string& modelPath, const std::string& animationPath = "") {
        // Load 3D model
        m_model = LoadModel(modelPath.c_str());
        if (m_model.meshCount == 0) {
            TraceLog(LOG_ERROR, "Failed to load model: %s", modelPath.c_str());
            return false;
        }

        m_modelLoaded = true;

        // Load animations if path provided
        if (!animationPath.empty()) {
            m_animations = LoadModelAnimations(animationPath.c_str(), &m_animationCount);
            if (m_animationCount > 0) {
                TraceLog(LOG_INFO, "Loaded %d animations from: %s", m_animationCount, animationPath.c_str());
            }
        }

        // Setup default animation clips based on common character animation structure
        setupDefaultClips();

        TraceLog(LOG_INFO, "Model loaded successfully: %s", modelPath.c_str());
        return true;
    }

    // Setup animation clips (frame ranges for different actions)
    void setupDefaultClips() {
        // These frame ranges are examples - adjust based on your actual animation files
        // Typically exported from Blender/Maya with specific frame ranges

        // Common animation frame ranges (adjust for your models)
        m_clips[AnimationType::IDLE] = AnimationClip("Idle", 0, 60, 1.0f, true);
        m_clips[AnimationType::WALK] = AnimationClip("Walk", 61, 91, 1.0f, true);
        m_clips[AnimationType::RUN] = AnimationClip("Run", 92, 112, 1.5f, true);
        m_clips[AnimationType::CROUCH] = AnimationClip("Crouch", 113, 143, 1.0f, true);
        m_clips[AnimationType::JUMP] = AnimationClip("Jump", 144, 164, 1.2f, false);
        m_clips[AnimationType::SHOOT] = AnimationClip("Shoot", 165, 175, 1.5f, false);
        m_clips[AnimationType::RELOAD] = AnimationClip("Reload", 176, 216, 1.0f, false);
        m_clips[AnimationType::MELEE] = AnimationClip("Melee", 217, 232, 1.3f, false);
        m_clips[AnimationType::HIT] = AnimationClip("Hit", 233, 243, 1.0f, false);
        m_clips[AnimationType::DIE] = AnimationClip("Die", 244, 284, 1.0f, false);
        m_clips[AnimationType::LOOT] = AnimationClip("Loot", 285, 325, 1.0f, false);
    }

    // Add or override animation clip
    void addClip(AnimationType type, const std::string& name, int start, int end, float speed = 1.0f, bool loop = true) {
        m_clips[type] = AnimationClip(name, start, end, speed, loop);
    }

    // Play an animation
    void play(AnimationType type, bool restart = false) {
        if (type == m_currentAnimation && !restart) return;

        m_currentAnimation = type;
        if (restart) {
            m_currentFrame = 0;
            m_frameAccumulator = 0.0f;
        }
        m_isPlaying = true;
    }

    // Update animation (call every frame)
    void update(float deltaTime) {
        if (!m_isPlaying || !m_modelLoaded || m_animationCount == 0) return;

        auto it = m_clips.find(m_currentAnimation);
        if (it == m_clips.end()) return;

        const AnimationClip& clip = it->second;

        // Accumulate time based on speed
        m_frameAccumulator += deltaTime * 30.0f * clip.speed; // Assuming 30 FPS animations

        if (m_frameAccumulator >= 1.0f) {
            int frameStep = static_cast<int>(m_frameAccumulator);
            m_frameAccumulator -= frameStep;

            m_currentFrame += frameStep;

            // Handle frame overflow
            int frameCount = clip.endFrame - clip.startFrame + 1;
            if (m_currentFrame >= frameCount) {
                if (clip.loop) {
                    m_currentFrame = m_currentFrame % frameCount;
                } else {
                    m_currentFrame = frameCount - 1;
                    m_isPlaying = false; // Stop at last frame
                }
            }
        }

        // Update model animation
        if (m_animationCount > 0 && m_animations != nullptr) {
            int actualFrame = clip.startFrame + m_currentFrame;
            UpdateModelAnimation(m_model, m_animations[0], actualFrame);
        }
    }

    // Draw the animated model
    void draw(Vector3 position, float scale, Color tint = WHITE) {
        if (!m_modelLoaded) return;
        DrawModel(m_model, position, scale, tint);
    }

    // Draw with rotation
    void draw(Vector3 position, Vector3 rotationAxis, float rotationAngle, float scale, Color tint = WHITE) {
        if (!m_modelLoaded) return;
        DrawModelEx(m_model, position, rotationAxis, rotationAngle, { scale, scale, scale }, tint);
    }

    // Check if current animation has finished (for non-looping animations)
    bool isAnimationFinished() const {
        auto it = m_clips.find(m_currentAnimation);
        if (it == m_clips.end()) return true;

        const AnimationClip& clip = it->second;
        int frameCount = clip.endFrame - clip.startFrame + 1;
        return !clip.loop && m_currentFrame >= frameCount - 1;
    }

    // Get current animation type
    AnimationType getCurrentAnimation() const { return m_currentAnimation; }

    // Check if playing
    bool isPlaying() const { return m_isPlaying; }

    // Stop animation
    void stop() { m_isPlaying = false; }

    // Resume animation
    void resume() { m_isPlaying = true; }

    // Get model reference (for custom rendering or physics)
    Model& getModel() { return m_model; }
    const Model& getModel() const { return m_model; }

    // Unload resources
    void unload() {
        if (m_animations != nullptr && m_animationCount > 0) {
            UnloadModelAnimations(m_animations, m_animationCount);
            m_animations = nullptr;
            m_animationCount = 0;
        }

        if (m_modelLoaded) {
            UnloadModel(m_model);
            m_modelLoaded = false;
        }
    }

    // Helper: Set texture for model
    void setTexture(Texture2D texture, int materialIndex = 0) {
        if (!m_modelLoaded || materialIndex >= m_model.materialCount) return;
        m_model.materials[materialIndex].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    }
};

// Player Animation State Machine
class PlayerAnimationStateMachine {
private:
    AnimationController& m_controller;
    AnimationType m_lastState;
    bool m_isDead;

public:
    PlayerAnimationStateMachine(AnimationController& controller)
        : m_controller(controller)
        , m_lastState(AnimationType::IDLE)
        , m_isDead(false)
    {
    }

    void update(float deltaTime, bool isMoving, bool isRunning, bool isCrouching,
                bool isShooting, bool isReloading, bool isLooting, bool isDead) {

        // Death has highest priority
        if (isDead && !m_isDead) {
            m_controller.play(AnimationType::DIE, true);
            m_isDead = true;
            return;
        }

        if (m_isDead) {
            m_controller.update(deltaTime);
            return; // Stay in death animation
        }

        // Determine animation based on state priority
        AnimationType newState = AnimationType::IDLE;

        if (isLooting) {
            newState = AnimationType::LOOT;
        }
        else if (isReloading) {
            newState = AnimationType::RELOAD;
        }
        else if (isShooting) {
            newState = AnimationType::SHOOT;
        }
        else if (isCrouching) {
            newState = AnimationType::CROUCH;
        }
        else if (isMoving) {
            newState = isRunning ? AnimationType::RUN : AnimationType::WALK;
        }

        // Only change if different from last state
        if (newState != m_lastState) {
            bool restart = (newState == AnimationType::SHOOT ||
                           newState == AnimationType::RELOAD ||
                           newState == AnimationType::LOOT);
            m_controller.play(newState, restart);
            m_lastState = newState;
        }

        m_controller.update(deltaTime);
    }

    void reset() {
        m_isDead = false;
        m_lastState = AnimationType::IDLE;
        m_controller.play(AnimationType::IDLE, true);
    }
};
