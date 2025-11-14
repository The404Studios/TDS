#pragma once

#include "raylib.h"
#include "common/Protocol.h"

namespace TDS {

class Game;

class Camera {
public:
    Camera(Game* game);
    ~Camera();

    void update(float dt);
    void reset();

    // Raylib camera access
    Camera3D& getCamera3D() { return camera; }
    const Camera3D& getCamera3D() const { return camera; }

    // Camera control
    void setPosition(const Vector3& pos);
    void setRotation(float yaw, float pitch);
    void setPitch(float newPitch);
    void setYaw(float newYaw);
    void setFOV(float fov);
    void moveForward(float amount);
    void moveRight(float amount);
    void rotate(float yawDelta, float pitchDelta);

    // Getters
    Vector3 getPosition() const;
    Vector3 getForward() const;
    Vector3 getRight() const;
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getFOV() const { return camera.fovy; }

private:
    void updateCameraVectors();

    Game* game;
    Camera3D camera;

    float yaw;
    float pitch;
    float mouseSensitivity;
    
    Vector3 forward;
    Vector3 right;
    Vector3 up;
};

} // namespace TDS
