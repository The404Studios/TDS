#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <cmath>

// Easing functions for smooth animations
enum class EasingType {
    LINEAR,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_IN_OUT_CUBIC,
    EASE_IN_QUART,
    EASE_OUT_QUART,
    EASE_IN_OUT_QUART,
    EASE_IN_SINE,
    EASE_OUT_SINE,
    EASE_IN_OUT_SINE,
    EASE_IN_EXPO,
    EASE_OUT_EXPO,
    EASE_IN_OUT_EXPO,
    EASE_IN_CIRC,
    EASE_OUT_CIRC,
    EASE_IN_OUT_CIRC,
    EASE_IN_BACK,
    EASE_OUT_BACK,
    EASE_IN_OUT_BACK,
    EASE_IN_ELASTIC,
    EASE_OUT_ELASTIC,
    EASE_IN_OUT_ELASTIC,
    EASE_IN_BOUNCE,
    EASE_OUT_BOUNCE,
    EASE_IN_OUT_BOUNCE
};

// Easing function calculator
class Easing {
public:
    static float apply(EasingType type, float t);

private:
    static float easeInQuad(float t);
    static float easeOutQuad(float t);
    static float easeInOutQuad(float t);
    static float easeInCubic(float t);
    static float easeOutCubic(float t);
    static float easeInOutCubic(float t);
    static float easeInQuart(float t);
    static float easeOutQuart(float t);
    static float easeInOutQuart(float t);
    static float easeInSine(float t);
    static float easeOutSine(float t);
    static float easeInOutSine(float t);
    static float easeInExpo(float t);
    static float easeOutExpo(float t);
    static float easeInOutExpo(float t);
    static float easeInCirc(float t);
    static float easeOutCirc(float t);
    static float easeInOutCirc(float t);
    static float easeInBack(float t);
    static float easeOutBack(float t);
    static float easeInOutBack(float t);
    static float easeInElastic(float t);
    static float easeOutElastic(float t);
    static float easeInOutElastic(float t);
    static float easeInBounce(float t);
    static float easeOutBounce(float t);
    static float easeInOutBounce(float t);
};

// Animation target - what property to animate
enum class AnimationTarget {
    POSITION_X,
    POSITION_Y,
    POSITION_Z,
    SCALE_X,
    SCALE_Y,
    ROTATION,
    ALPHA,
    COLOR_R,
    COLOR_G,
    COLOR_B,
    WIDTH,
    HEIGHT,
    CUSTOM
};

// Animation base class
class Animation {
public:
    Animation(float duration, EasingType easing = EasingType::LINEAR);
    virtual ~Animation() = default;

    // Update animation
    virtual void update(float deltaTime);
    virtual void apply() = 0;

    // Control
    void start();
    void pause();
    void resume();
    void stop();
    void reset();

    // Properties
    bool isPlaying() const { return playing && !paused; }
    bool isFinished() const { return currentTime >= duration; }
    bool isPaused() const { return paused; }
    float getProgress() const { return currentTime / duration; }
    float getEasedProgress() const { return Easing::apply(easingType, getProgress()); }

    // Callbacks
    void setOnComplete(std::function<void()> callback) { onComplete = callback; }
    void setOnUpdate(std::function<void(float)> callback) { onUpdate = callback; }

    // Settings
    void setDuration(float dur) { duration = dur; }
    float getDuration() const { return duration; }
    void setEasing(EasingType easing) { easingType = easing; }
    void setLoop(bool loop) { looping = loop; }
    void setReverse(bool rev) { reverse = rev; }

protected:
    float duration;
    float currentTime;
    EasingType easingType;
    bool playing;
    bool paused;
    bool looping;
    bool reverse;

    std::function<void()> onComplete;
    std::function<void(float)> onUpdate;
};

// Float value animation
class FloatAnimation : public Animation {
public:
    FloatAnimation(float* target, float from, float to, float duration, EasingType easing = EasingType::LINEAR);

    void apply() override;

private:
    float* target;
    float fromValue;
    float toValue;
};

// Transform animation (for GameObject)
class TransformAnimation : public Animation {
public:
    TransformAnimation(class Transform* transform, AnimationTarget target,
                      float from, float to, float duration, EasingType easing = EasingType::LINEAR);

    void apply() override;

private:
    class Transform* transform;
    AnimationTarget animTarget;
    float fromValue;
    float toValue;
};

// Color animation
struct Color;
class ColorAnimation : public Animation {
public:
    ColorAnimation(Color* target, const Color& from, const Color& to,
                  float duration, EasingType easing = EasingType::LINEAR);

    void apply() override;

private:
    Color* target;
    Color fromColor;
    Color toColor;
};

// Animation sequence - play animations one after another
class AnimationSequence {
public:
    AnimationSequence();

    void addAnimation(std::shared_ptr<Animation> anim);
    void update(float deltaTime);

    bool isPlaying() const { return playing; }
    bool isFinished() const;
    void start();
    void stop();
    void reset();

    void setLoop(bool loop) { looping = loop; }
    void setOnComplete(std::function<void()> callback) { onComplete = callback; }

private:
    std::vector<std::shared_ptr<Animation>> animations;
    size_t currentIndex;
    bool playing;
    bool looping;
    std::function<void()> onComplete;
};

// Animation group - play animations simultaneously
class AnimationGroup {
public:
    AnimationGroup();

    void addAnimation(std::shared_ptr<Animation> anim);
    void update(float deltaTime);

    bool isPlaying() const { return playing; }
    bool isFinished() const;
    void start();
    void stop();
    void reset();

    void setOnComplete(std::function<void()> callback) { onComplete = callback; }

private:
    std::vector<std::shared_ptr<Animation>> animations;
    bool playing;
    std::function<void()> onComplete;
};

// Animation manager - manages all active animations
class AnimationManager {
public:
    static AnimationManager& getInstance();

    void update(float deltaTime);

    // Add animations
    void addAnimation(std::shared_ptr<Animation> anim);
    void addSequence(std::shared_ptr<AnimationSequence> seq);
    void addGroup(std::shared_ptr<AnimationGroup> group);

    // Clear
    void clearAll();
    void removeFinished();

    // Stats
    size_t getActiveCount() const;

private:
    AnimationManager() = default;
    std::vector<std::shared_ptr<Animation>> animations;
    std::vector<std::shared_ptr<AnimationSequence>> sequences;
    std::vector<std::shared_ptr<AnimationGroup>> groups;
};
