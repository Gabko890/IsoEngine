#pragma once
#include <glm/glm.hpp>

class ICamera {
public:
    virtual ~ICamera() = default;
    virtual glm::mat4 GetViewMatrix() const = 0;
};