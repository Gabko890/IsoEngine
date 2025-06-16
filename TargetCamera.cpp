#include "TargetCamera.hpp"

TargetCamera::TargetCamera(glm::vec3 pos, glm::vec3 tgt, glm::vec3 upVec)
    : position(pos), target(tgt), up(upVec) {}

glm::mat4 TargetCamera::GetViewMatrix() const {
    return glm::lookAt(position, target, up);
}

glm::vec3 TargetCamera::GetPosition() const {
    return position;
}

glm::vec3 TargetCamera::GetDirection() const {
    return glm::normalize(target - position);
}