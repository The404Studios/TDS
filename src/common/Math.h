#pragma once

#include "Protocol.h"
#include <cmath>
#include <algorithm>

// Undefine raylib macros to avoid conflicts with our constexpr
#ifdef PI
#undef PI
#endif
#ifdef DEG2RAD
#undef DEG2RAD
#endif
#ifdef RAD2DEG
#undef RAD2DEG
#endif

namespace TDS {
namespace Math {

// Constants
constexpr float PI = 3.14159265358979323846f;
constexpr float DEG2RAD = PI / 180.0f;
constexpr float RAD2DEG = 180.0f / PI;

// Vector3 operations
inline float dot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vector3 cross(const Vector3& a, const Vector3& b) {
    return Vector3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

inline float length(const Vector3& v) {
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline float lengthSquared(const Vector3& v) {
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

inline float distance(const Vector3& a, const Vector3& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

inline float distanceSquared(const Vector3& a, const Vector3& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return dx * dx + dy * dy + dz * dz;
}

inline Vector3 normalize(const Vector3& v) {
    float len = length(v);
    if (len > 0.0001f) {
        return Vector3(v.x / len, v.y / len, v.z / len);
    }
    return Vector3(0, 0, 0);
}

inline Vector3 add(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline Vector3 subtract(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}

inline Vector3 scale(const Vector3& v, float s) {
    return Vector3(v.x * s, v.y * s, v.z * s);
}

inline Vector3 lerp(const Vector3& a, const Vector3& b, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return Vector3(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    );
}

// Utility functions
inline float clamp(float value, float min, float max) {
    return std::max(min, std::min(value, max));
}

inline float lerp(float a, float b, float t) {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

// Wrap angle to [-180, 180]
inline float wrapAngle(float angle) {
    while (angle > 180.0f) angle -= 360.0f;
    while (angle < -180.0f) angle += 360.0f;
    return angle;
}

// Shortest angle difference
inline float angleDifference(float a, float b) {
    float diff = wrapAngle(b - a);
    return diff;
}

// Raycast structure
struct Ray {
    Vector3 origin;
    Vector3 direction;  // Should be normalized

    Ray() {}
    Ray(const Vector3& origin, const Vector3& direction)
        : origin(origin), direction(direction) {}

    Vector3 pointAt(float t) const {
        return add(origin, scale(direction, t));
    }
};

// Axis-aligned bounding box
struct AABB {
    Vector3 min;
    Vector3 max;

    AABB() {}
    AABB(const Vector3& min, const Vector3& max) : min(min), max(max) {}

    bool contains(const Vector3& point) const {
        return point.x >= min.x && point.x <= max.x &&
               point.y >= min.y && point.y <= max.y &&
               point.z >= min.z && point.z <= max.z;
    }

    bool intersects(const AABB& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
               (min.y <= other.max.y && max.y >= other.min.y) &&
               (min.z <= other.max.z && max.z >= other.min.z);
    }

    Vector3 center() const {
        return Vector3(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f,
            (min.z + max.z) * 0.5f
        );
    }

    Vector3 size() const {
        return Vector3(
            max.x - min.x,
            max.y - min.y,
            max.z - min.z
        );
    }
};

// Ray-AABB intersection
inline bool rayIntersectsAABB(const Ray& ray, const AABB& box, float& tmin, float& tmax) {
    tmin = 0.0f;
    tmax = 1000000.0f;

    for (int i = 0; i < 3; i++) {
        float origin = (&ray.origin.x)[i];
        float dir = (&ray.direction.x)[i];
        float boxMin = (&box.min.x)[i];
        float boxMax = (&box.max.x)[i];

        if (std::abs(dir) < 0.0001f) {
            if (origin < boxMin || origin > boxMax)
                return false;
        } else {
            float t1 = (boxMin - origin) / dir;
            float t2 = (boxMax - origin) / dir;

            if (t1 > t2) std::swap(t1, t2);

            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);

            if (tmin > tmax) return false;
        }
    }

    return true;
}

// Generate random float [0, 1]
inline float randomFloat() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

// Generate random float [min, max]
inline float randomFloat(float min, float max) {
    return min + randomFloat() * (max - min);
}

// Generate random Vector3 in sphere
inline Vector3 randomInSphere(float radius) {
    Vector3 v;
    do {
        v = Vector3(
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f),
            randomFloat(-1.0f, 1.0f)
        );
    } while (lengthSquared(v) > 1.0f);
    return scale(v, radius);
}

} // namespace Math
} // namespace TDS
