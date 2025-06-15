#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

struct MeshPrimitive {
    unsigned int vao = 0;
    unsigned int vbo = 0;
    unsigned int ebo = 0;
    size_t indexCount = 0;
    GLuint texture = 0;
    std::string name;
};