#include "Camera.h"
#include "../Game.h"
#include "raymath.h"

namespace TDS {

Camera::Camera(Game* game)
    : game(game)
    , yaw(0.0f)
    , pitch(0.0f)
    , mouseSensitivity(0.002f)
{
    camera.position = (::Vector3){ 0.0f, 1.7f, 0.0f };
    camera.target = (::Vector3){ 0.0f, 1.7f, -1.0f };
    camera.up = (::Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 70.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    updateCameraVectors();
}

Camera::~Camera() {}

void Camera::update(float dt) {
    // Mouse look
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 delta = GetMouseDelta();
        rotate(delta.x * mouseSensitivity, -delta.y * mouseSensitivity);
    }

    updateCameraVectors();
}

void Camera::reset() {
    yaw = 0.0f;
    pitch = 0.0f;
    camera.position = (::Vector3){ 0.0f, 1.7f, 0.0f };
    updateCameraVectors();
}

void Camera::setPosition(const TDS::Vector3& pos) {
    camera.position = (::Vector3){ pos.x, pos.y, pos.z };
    updateCameraVectors();
}

void Camera::setRotation(float y, float p) {
    yaw = y;
    pitch = p;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    updateCameraVectors();
}

void Camera::setPitch(float newPitch) {
    pitch = newPitch;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    updateCameraVectors();
}

void Camera::setYaw(float newYaw) {
    yaw = newYaw;
    updateCameraVectors();
}

void Camera::setFOV(float fov) {
    camera.fovy = fov;
}

void Camera::moveForward(float amount) {
    camera.position.x += forward.x * amount;
    camera.position.z += forward.z * amount;
    updateCameraVectors();
}

void Camera::moveRight(float amount) {
    camera.position.x += right.x * amount;
    camera.position.z += right.z * amount;
    updateCameraVectors();
}

void Camera::rotate(float yawDelta, float pitchDelta) {
    yaw += yawDelta;
    pitch += pitchDelta;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    updateCameraVectors();
}

TDS::Vector3 Camera::getPosition() const {
    return TDS::Vector3(camera.position.x, camera.position.y, camera.position.z);
}

TDS::Vector3 Camera::getForward() const {
    return forward;
}

TDS::Vector3 Camera::getRight() const {
    return right;
}

void Camera::updateCameraVectors() {
    // Calculate forward vector
    forward.x = cos(yaw * DEG2RAD) * cos(pitch * DEG2RAD);
    forward.y = sin(pitch * DEG2RAD);
    forward.z = sin(yaw * DEG2RAD) * cos(pitch * DEG2RAD);

    // Normalize
    float len = sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    if (len > 0.001f) {
        forward.x /= len;
        forward.y /= len;
        forward.z /= len;
    }

    // Calculate right vector
    right.x = forward.z;
    right.y = 0.0f;
    right.z = -forward.x;
    len = sqrt(right.x * right.x + right.z * right.z);
    if (len > 0.001f) {
        right.x /= len;
        right.z /= len;
    }

    // Up vector
    up.x = 0.0f;
    up.y = 1.0f;
    up.z = 0.0f;

    // Update Raylib camera target
    camera.target.x = camera.position.x + forward.x;
    camera.target.y = camera.position.y + forward.y;
    camera.target.z = camera.position.z + forward.z;
}

} // namespace TDS
