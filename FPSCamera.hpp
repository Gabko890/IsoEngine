#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ICamera.hpp"

class FPSCamera : public ICamera {
public:
    glm::vec3 position;
    float pitch, yaw;

    FPSCamera(glm::vec3 startPos = { 0, 0, 3 }, float startPitch = 0.0f, float startYaw = -90.0f);

    glm::mat4 GetViewMatrix() const override;
    glm::vec3 GetPosition() const override;
    glm::vec3 GetDirection() const override;

    void Move(const glm::vec3& delta, float speed);

    void MoveForward(float delta, float speed);

    void MoveRight(float delta, float speed);

    void MoveUp(float delta, float speed);

    void Rotate(float deltaPitch, float deltaYaw);

    void SetPosition(const glm::vec3& newPos);

    void SetOrientation(float newPitch, float newYaw);

    void SetPose(const glm::vec3& newPos, float newPitch, float newYaw);
};