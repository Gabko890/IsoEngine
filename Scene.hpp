#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "ModelInstance.hpp"

class ICamera;
class Renderer;

namespace tinygltf {
    class Model;
    struct Primitive;
}

struct SceneObject {
    std::string id;
    std::string modelPath;
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    glm::vec3 scale = glm::vec3(1.0f);
    std::vector<ModelInstance> instances;

    glm::mat4 GetTransform() const;

    // Binary serialization
    void WriteToBinary(std::ofstream& file) const;
    bool ReadFromBinary(std::ifstream& file);
};

class Scene {
public:
    Scene();

    bool AddObject(const std::string& id, const std::string& modelPath);
    bool RemoveObject(const std::string& id);

    SceneObject* GetObject(const std::string& id);
    const SceneObject* GetObject(const std::string& id) const;

    void SetObjectPosition(const std::string& id, const glm::vec3& position);
    void SetObjectRotation(const std::string& id, const glm::vec3& rotation);
    void SetObjectScale(const std::string& id, const glm::vec3& scale);

    void SetBGColor(float r, float g, float b);
    void AddPathAlias(std::string key, std::string value);

    void MoveObject(const std::string& id, const glm::vec3& offset);
    void RotateObject(const std::string& id, const glm::vec3& rotation);
    void ScaleObject(const std::string& id, const glm::vec3& scale);

    void RenderScene(Renderer& renderer, const ICamera& camera) const;

    const std::unordered_map<std::string, SceneObject>& GetObjects() const { return objects; }

    bool SaveToFile(const std::string& filePath) const;
    bool LoadFromFile(const std::string& filePath);

private:
    std::unordered_map<std::string, SceneObject> objects;
    std::vector<std::pair<std::string, std::string>> path_aliases;
    glm::vec3 bg_color;

    bool LoadModel(const std::string& path, std::vector<ModelInstance>& instances);
    bool LoadPrimitive(const std::string path, const tinygltf::Model& model, const tinygltf::Primitive& primitive, MeshPrimitive& meshPrim);
};