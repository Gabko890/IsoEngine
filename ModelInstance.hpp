#pragma once

#include "MeshPrimitive.hpp"
#include <glm/glm.hpp>

struct ModelInstance {
    glm::mat4 transform;
    MeshPrimitive* mesh;
};