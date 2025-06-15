#pragma once

#include <string>
#include <vector>
#include <tiny_gltf.h>
#include <glm/glm.hpp>

#include "ModelInstance.hpp"

class GLTFLoader {
public:
    bool LoadModel(const std::string& path);
    const std::vector<ModelInstance>& GetInstances() const;

private:
    bool LoadPrimitive(const tinygltf::Model& model, const tinygltf::Primitive& primitive, MeshPrimitive& meshPrim);
    std::vector<ModelInstance> instances;
};