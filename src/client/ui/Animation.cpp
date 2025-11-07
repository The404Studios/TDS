#include "Animation.h"
#include "GameObject.h"
#include "UIElement.h"
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// ===== Easing Functions =====

float Easing::apply(EasingType type, float t) {
    t = std::max(0.0f, std::min(1.0f, t));  // Clamp to [0, 1]

    switch (type) {
        case EasingType::LINEAR: return t;
        case EasingType::EASE_IN_QUAD: return easeInQuad(t);
        case EasingType::EASE_OUT_QUAD: return easeOutQuad(t);
        case EasingType::EASE_IN_OUT_QUAD: return easeInOutQuad(t);
        case EasingType::EASE_IN_CUBIC: return easeInCubic(t);
        case EasingType::EASE_OUT_CUBIC: return easeOutCubic(t);
        case EasingType::EASE_IN_OUT_CUBIC: return easeInOutCubic(t);
        case EasingType::EASE_IN_QUART: return easeInQuart(t);
        case EasingType::EASE_OUT_QUART: return easeOutQuart(t);
        case EasingType::EASE_IN_OUT_QUART: return easeInOutQuart(t);
        case EasingType::EASE_IN_SINE: return easeInSine(t);
        case EasingType::EASE_OUT_SINE: return easeOutSine(t);
        case EasingType::EASE_IN_OUT_SINE: return easeInOutSine(t);
        case EasingType::EASE_IN_EXPO: return easeInExpo(t);
        case EasingType::EASE_OUT_EXPO: return easeOutExpo(t);
        case EasingType::EASE_IN_OUT_EXPO: return easeInOutExpo(t);
        case EasingType::EASE_IN_CIRC: return easeInCirc(t);
        case EasingType::EASE_OUT_CIRC: return easeOutCirc(t);
        case EasingType::EASE_IN_OUT_CIRC: return easeInOutCirc(t);
        case EasingType::EASE_IN_BACK: return easeInBack(t);
        case EasingType::EASE_OUT_BACK: return easeOutBack(t);
        case EasingType::EASE_IN_OUT_BACK: return easeInOutBack(t);
        case EasingType::EASE_IN_ELASTIC: return easeInElastic(t);
        case EasingType::EASE_OUT_ELASTIC: return easeOutElastic(t);
        case EasingType::EASE_IN_OUT_ELASTIC: return easeInOutElastic(t);
        case EasingType::EASE_IN_BOUNCE: return easeInBounce(t);
        case EasingType::EASE_OUT_BOUNCE: return easeOutBounce(t);
        case EasingType::EASE_IN_OUT_BOUNCE: return easeInOutBounce(t);
        default: return t;
    }
}

float Easing::easeInQuad(float t) { return t * t; }
float Easing::easeOutQuad(float t) { return t * (2.0f - t); }
float Easing::easeInOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

float Easing::easeInCubic(float t) { return t * t * t; }
float Easing::easeOutCubic(float t) {
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}
float Easing::easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
}

float Easing::easeInQuart(float t) { return t * t * t * t; }
float Easing::easeOutQuart(float t) {
    float f = t - 1.0f;
    return 1.0f - f * f * f * f;
}
float Easing::easeInOutQuart(float t) {
    if (t < 0.5f) {
        return 8.0f * t * t * t * t;
    } else {
        float f = t - 1.0f;
        return 1.0f - 8.0f * f * f * f * f;
    }
}

float Easing::easeInSine(float t) {
    return 1.0f - std::cosf(t * M_PI / 2.0f);
}
float Easing::easeOutSine(float t) {
    return std::sinf(t * M_PI / 2.0f);
}
float Easing::easeInOutSine(float t) {
    return -(std::cosf(M_PI * t) - 1.0f) / 2.0f;
}

float Easing::easeInExpo(float t) {
    return t == 0.0f ? 0.0f : std::powf(2.0f, 10.0f * (t - 1.0f));
}
float Easing::easeOutExpo(float t) {
    return t == 1.0f ? 1.0f : 1.0f - std::powf(2.0f, -10.0f * t);
}
float Easing::easeInOutExpo(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    if (t < 0.5f) {
        return std::powf(2.0f, 20.0f * t - 10.0f) / 2.0f;
    } else {
        return (2.0f - std::powf(2.0f, -20.0f * t + 10.0f)) / 2.0f;
    }
}

float Easing::easeInCirc(float t) {
    return 1.0f - std::sqrtf(1.0f - t * t);
}
float Easing::easeOutCirc(float t) {
    return std::sqrtf(1.0f - (t - 1.0f) * (t - 1.0f));
}
float Easing::easeInOutCirc(float t) {
    if (t < 0.5f) {
        return (1.0f - std::sqrtf(1.0f - 4.0f * t * t)) / 2.0f;
    } else {
        return (std::sqrtf(1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f)) + 1.0f) / 2.0f;
    }
}

float Easing::easeInBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return c3 * t * t * t - c1 * t * t;
}
float Easing::easeOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * std::powf(t - 1.0f, 3.0f) + c1 * std::powf(t - 1.0f, 2.0f);
}
float Easing::easeInOutBack(float t) {
    const float c1 = 1.70158f;
    const float c2 = c1 * 1.525f;
    if (t < 0.5f) {
        return (std::powf(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f;
    } else {
        return (std::powf(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }
}

float Easing::easeInElastic(float t) {
    const float c4 = (2.0f * M_PI) / 3.0f;
    if (t == 0.0f || t == 1.0f) return t;
    return -std::powf(2.0f, 10.0f * t - 10.0f) * std::sinf((t * 10.0f - 10.75f) * c4);
}
float Easing::easeOutElastic(float t) {
    const float c4 = (2.0f * M_PI) / 3.0f;
    if (t == 0.0f || t == 1.0f) return t;
    return std::powf(2.0f, -10.0f * t) * std::sinf((t * 10.0f - 0.75f) * c4) + 1.0f;
}
float Easing::easeInOutElastic(float t) {
    const float c5 = (2.0f * M_PI) / 4.5f;
    if (t == 0.0f || t == 1.0f) return t;
    if (t < 0.5f) {
        return -(std::powf(2.0f, 20.0f * t - 10.0f) * std::sinf((20.0f * t - 11.125f) * c5)) / 2.0f;
    } else {
        return (std::powf(2.0f, -20.0f * t + 10.0f) * std::sinf((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
    }
}

float Easing::easeOutBounce(float t) {
    const float n1 = 7.5625f;
    const float d1 = 2.75f;
    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}
float Easing::easeInBounce(float t) {
    return 1.0f - easeOutBounce(1.0f - t);
}
float Easing::easeInOutBounce(float t) {
    if (t < 0.5f) {
        return (1.0f - easeOutBounce(1.0f - 2.0f * t)) / 2.0f;
    } else {
        return (1.0f + easeOutBounce(2.0f * t - 1.0f)) / 2.0f;
    }
}

// ===== Animation Base =====

Animation::Animation(float duration, EasingType easing)
    : duration(duration), currentTime(0.0f), easingType(easing),
      playing(false), paused(false), looping(false), reverse(false),
      onComplete(nullptr), onUpdate(nullptr) {
}

void Animation::start() {
    playing = true;
    paused = false;
    currentTime = 0.0f;
}

void Animation::pause() {
    paused = true;
}

void Animation::resume() {
    paused = false;
}

void Animation::stop() {
    playing = false;
    paused = false;
}

void Animation::reset() {
    currentTime = 0.0f;
}

void Animation::update(float deltaTime) {
    if (!playing || paused) return;

    currentTime += deltaTime;

    if (currentTime >= duration) {
        if (looping) {
            currentTime = std::fmod(currentTime, duration);
        } else {
            currentTime = duration;
            playing = false;
        }

        if (onComplete && !looping) {
            onComplete();
        }
    }

    apply();

    if (onUpdate) {
        onUpdate(getEasedProgress());
    }
}

// ===== Float Animation =====

FloatAnimation::FloatAnimation(float* target, float from, float to, float duration, EasingType easing)
    : Animation(duration, easing), target(target), fromValue(from), toValue(to) {
}

void FloatAnimation::apply() {
    if (!target) return;
    float progress = getEasedProgress();
    *target = fromValue + (toValue - fromValue) * progress;
}

// ===== Transform Animation =====

TransformAnimation::TransformAnimation(Transform* transform, AnimationTarget target,
                                     float from, float to, float duration, EasingType easing)
    : Animation(duration, easing), transform(transform), animTarget(target),
      fromValue(from), toValue(to) {
}

void TransformAnimation::apply() {
    if (!transform) return;
    float progress = getEasedProgress();
    float value = fromValue + (toValue - fromValue) * progress;

    switch (animTarget) {
        case AnimationTarget::POSITION_X:
            transform->x = value;
            break;
        case AnimationTarget::POSITION_Y:
            transform->y = value;
            break;
        case AnimationTarget::POSITION_Z:
            transform->z = value;
            break;
        case AnimationTarget::SCALE_X:
            transform->scaleX = value;
            break;
        case AnimationTarget::SCALE_Y:
            transform->scaleY = value;
            break;
        case AnimationTarget::ROTATION:
            transform->rotation = value;
            break;
        default:
            break;
    }
}

// ===== Color Animation =====

ColorAnimation::ColorAnimation(Color* target, const Color& from, const Color& to,
                             float duration, EasingType easing)
    : Animation(duration, easing), target(target), fromColor(from), toColor(to) {
}

void ColorAnimation::apply() {
    if (!target) return;
    float progress = getEasedProgress();
    target->r = fromColor.r + (toColor.r - fromColor.r) * progress;
    target->g = fromColor.g + (toColor.g - fromColor.g) * progress;
    target->b = fromColor.b + (toColor.b - fromColor.b) * progress;
    target->a = fromColor.a + (toColor.a - fromColor.a) * progress;
}

// ===== Animation Sequence =====

AnimationSequence::AnimationSequence()
    : currentIndex(0), playing(false), looping(false), onComplete(nullptr) {
}

void AnimationSequence::addAnimation(std::shared_ptr<Animation> anim) {
    animations.push_back(anim);
}

void AnimationSequence::start() {
    if (animations.empty()) return;
    playing = true;
    currentIndex = 0;
    animations[currentIndex]->start();
}

void AnimationSequence::stop() {
    playing = false;
    if (currentIndex < animations.size()) {
        animations[currentIndex]->stop();
    }
}

void AnimationSequence::reset() {
    currentIndex = 0;
    for (auto& anim : animations) {
        anim->reset();
    }
}

void AnimationSequence::update(float deltaTime) {
    if (!playing || animations.empty()) return;

    auto& currentAnim = animations[currentIndex];
    currentAnim->update(deltaTime);

    if (currentAnim->isFinished()) {
        currentIndex++;
        if (currentIndex >= animations.size()) {
            if (looping) {
                reset();
                start();
            } else {
                playing = false;
                if (onComplete) {
                    onComplete();
                }
            }
        } else {
            animations[currentIndex]->start();
        }
    }
}

bool AnimationSequence::isFinished() const {
    return currentIndex >= animations.size();
}

// ===== Animation Group =====

AnimationGroup::AnimationGroup()
    : playing(false), onComplete(nullptr) {
}

void AnimationGroup::addAnimation(std::shared_ptr<Animation> anim) {
    animations.push_back(anim);
}

void AnimationGroup::start() {
    if (animations.empty()) return;
    playing = true;
    for (auto& anim : animations) {
        anim->start();
    }
}

void AnimationGroup::stop() {
    playing = false;
    for (auto& anim : animations) {
        anim->stop();
    }
}

void AnimationGroup::reset() {
    for (auto& anim : animations) {
        anim->reset();
    }
}

void AnimationGroup::update(float deltaTime) {
    if (!playing || animations.empty()) return;

    for (auto& anim : animations) {
        anim->update(deltaTime);
    }

    if (isFinished()) {
        playing = false;
        if (onComplete) {
            onComplete();
        }
    }
}

bool AnimationGroup::isFinished() const {
    for (const auto& anim : animations) {
        if (!anim->isFinished()) {
            return false;
        }
    }
    return !animations.empty();
}

// ===== Animation Manager =====

AnimationManager& AnimationManager::getInstance() {
    static AnimationManager instance;
    return instance;
}

void AnimationManager::update(float deltaTime) {
    // Update all animations
    for (auto& anim : animations) {
        if (anim) {
            anim->update(deltaTime);
        }
    }

    // Update sequences
    for (auto& seq : sequences) {
        if (seq) {
            seq->update(deltaTime);
        }
    }

    // Update groups
    for (auto& group : groups) {
        if (group) {
            group->update(deltaTime);
        }
    }

    // Remove finished animations
    removeFinished();
}

void AnimationManager::addAnimation(std::shared_ptr<Animation> anim) {
    if (anim) {
        animations.push_back(anim);
    }
}

void AnimationManager::addSequence(std::shared_ptr<AnimationSequence> seq) {
    if (seq) {
        sequences.push_back(seq);
    }
}

void AnimationManager::addGroup(std::shared_ptr<AnimationGroup> group) {
    if (group) {
        groups.push_back(group);
    }
}

void AnimationManager::clearAll() {
    animations.clear();
    sequences.clear();
    groups.clear();
}

void AnimationManager::removeFinished() {
    animations.erase(
        std::remove_if(animations.begin(), animations.end(),
            [](const std::shared_ptr<Animation>& anim) {
                return anim->isFinished() && !anim->isPlaying();
            }),
        animations.end()
    );

    sequences.erase(
        std::remove_if(sequences.begin(), sequences.end(),
            [](const std::shared_ptr<AnimationSequence>& seq) {
                return seq->isFinished() && !seq->isPlaying();
            }),
        sequences.end()
    );

    groups.erase(
        std::remove_if(groups.begin(), groups.end(),
            [](const std::shared_ptr<AnimationGroup>& group) {
                return group->isFinished() && !group->isPlaying();
            }),
        groups.end()
    );
}

size_t AnimationManager::getActiveCount() const {
    return animations.size() + sequences.size() + groups.size();
}
