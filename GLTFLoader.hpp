#pragma once

#include <tiny_gltf.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

struct MeshPrimitive {
    unsigned int vao;
    unsigned int vbo, ebo;
    size_t indexCount;
    GLuint texture = 0;
    // material ID, transform
};

class GLTFLoader {
private:
    tinygltf::Model model;
    std::vector<MeshPrimitive> primitives;

public:
    GLTFLoader();
    ~GLTFLoader();

    bool LoadModel(const std::string& path);
    const std::vector<MeshPrimitive>& GetPrimitives() const;
};
