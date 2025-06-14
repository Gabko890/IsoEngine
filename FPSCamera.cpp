#include "FPSCamera.hpp"

FPSCamera::FPSCamera(glm::vec3 startPos, float startPitch, float startYaw)
    : position(startPos), pitch(startPitch), yaw(startYaw) {}

glm::mat4 FPSCamera::GetViewMatrix() const {
    glm::vec3 front = GetDirection();
    return glm::lookAt(position, position + front, glm::vec3(0, 1, 0));
}

void FPSCamera::Move(const glm::vec3& delta, float speed) {
    glm::vec3 forward = GetDirection();
    glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up = glm::vec3(0, 1, 0);

    position += right * delta.x * speed;
    position += up * delta.y * speed;
    position += forward * delta.z * speed;
}

void FPSCamera::MoveForward(float delta, float speed) {
    glm::vec3 forward = GetDirection();
    position += forward * delta * speed;
}

void FPSCamera:: MoveRight(float delta, float speed) {
    glm::vec3 right = glm::normalize(glm::cross(GetDirection(), glm::vec3(0, 1, 0)));
    position += right * delta * speed;
}

void FPSCamera::MoveUp(float delta, float speed) {
    position.y += delta * speed;
}

void FPSCamera::Rotate(float deltaPitch, float deltaYaw) {
    pitch += deltaPitch;
    yaw += deltaYaw;
    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

glm::vec3 FPSCamera::GetDirection() const {
    glm::vec3 direction;
    direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    direction.y = sin(glm::radians(pitch));
    direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    return glm::normalize(direction);
}

void FPSCamera::SetPosition(const glm::vec3& newPos) {
    position = newPos;
}

void FPSCamera::SetOrientation(float newPitch, float newYaw) {
    pitch = glm::clamp(newPitch, -89.0f, 89.0f);
    yaw = newYaw;
}

void FPSCamera::SetPose(const glm::vec3& newPos, float newPitch, float newYaw) {
    SetPosition(newPos);
    SetOrientation(newPitch, newYaw);
}