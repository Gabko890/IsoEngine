#pragma once

#include "MeshPrimitive.hpp"
#include <unordered_map>
#include <string>

class ResourceManager {
public:
    static MeshPrimitive* GetOrCreateMesh(const std::string& key, const MeshPrimitive& mesh);
    static GLuint GetOrCreateTexture(const std::string& key, GLuint texture);

    static void Clear();

private:
    static std::unordered_map<std::string, MeshPrimitive> meshCache;
    static std::unordered_map<std::string, GLuint> textureCache;
};