#include "ResourceManager.hpp"
#include <glad/glad.h>

std::unordered_map<std::string, MeshPrimitive> ResourceManager::meshCache;
std::unordered_map<std::string, GLuint> ResourceManager::textureCache;

MeshPrimitive* ResourceManager::GetOrCreateMesh(const std::string& key, const MeshPrimitive& mesh) {
    auto it = meshCache.find(key);
    if (it != meshCache.end()) {
        return &it->second;
    }

    auto result = meshCache.emplace(key, mesh);
    return &result.first->second;
}

GLuint ResourceManager::GetOrCreateTexture(const std::string& key, GLuint texture) {
    auto it = textureCache.find(key);
    if (it != textureCache.end()) {
        glDeleteTextures(1, &texture);
        return it->second;
    }

    textureCache[key] = texture;
    return texture;
}

void ResourceManager::Clear() {
    for (auto& pair : meshCache) {
        const MeshPrimitive& mesh = pair.second;
        glDeleteVertexArrays(1, &mesh.vao);
        glDeleteBuffers(1, &mesh.vbo);
        glDeleteBuffers(1, &mesh.ebo);
    }
    meshCache.clear();

    for (auto& pair : textureCache) {
        glDeleteTextures(1, &pair.second);
    }
    textureCache.clear();
}