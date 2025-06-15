#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ICamera.hpp"

class TargetCamera : public ICamera {
public:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;

    TargetCamera(glm::vec3 pos, glm::vec3 tgt, glm::vec3 upVec = glm::vec3(0, 1, 0));

    glm::mat4 GetViewMatrix() const override;
};