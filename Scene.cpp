#include "Scene.hpp"
#include "ResourceManager.hpp"
#include "Renderer.hpp"
#include "ICamera.hpp"

#include "Utils.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <tiny_gltf.h>
#include <glad/glad.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstring>


Scene::Scene() :
    bg_color(30 / 255.0f, 30 / 255.0f, 30 / 255.0f)
{
    path_aliases.emplace_back("assets", "../../assets");
}

glm::mat4 SceneObject::GetTransform() const {
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, position);
    transform = glm::rotate(transform, rotation.x, glm::vec3(1, 0, 0));
    transform = glm::rotate(transform, rotation.y, glm::vec3(0, 1, 0));
    transform = glm::rotate(transform, rotation.z, glm::vec3(0, 0, 1));
    transform = glm::scale(transform, scale);
    return transform;
}

void SceneObject::WriteToBinary(std::ofstream& file) const {
    // Write string lengths and data
    size_t idLen = id.length();
    file.write(reinterpret_cast<const char*>(&idLen), sizeof(idLen));
    file.write(id.c_str(), idLen);

    size_t pathLen = modelPath.length();
    file.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
    file.write(modelPath.c_str(), pathLen);

    // Write transform data
    file.write(reinterpret_cast<const char*>(&position), sizeof(position));
    file.write(reinterpret_cast<const char*>(&rotation), sizeof(rotation));
    file.write(reinterpret_cast<const char*>(&scale), sizeof(scale));
}

bool SceneObject::ReadFromBinary(std::ifstream& file) {
    try {
        // Read ID
        size_t idLen;
        if (!file.read(reinterpret_cast<char*>(&idLen), sizeof(idLen))) return false;
        if (idLen > 1024) return false; // Sanity check

        id.resize(idLen);
        if (!file.read(&id[0], idLen)) return false;

        // Read model path
        size_t pathLen;
        if (!file.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen))) return false;
        if (pathLen > 2048) return false; // Sanity check

        modelPath.resize(pathLen);
        if (!file.read(&modelPath[0], pathLen)) return false;

        // Read transform data
        if (!file.read(reinterpret_cast<char*>(&position), sizeof(position))) return false;
        if (!file.read(reinterpret_cast<char*>(&rotation), sizeof(rotation))) return false;
        if (!file.read(reinterpret_cast<char*>(&scale), sizeof(scale))) return false;

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error reading SceneObject from binary: " << e.what() << std::endl;
        return false;
    }
}

bool Scene::AddObject(const std::string& id, const std::string& modelPath) {
    if (objects.find(id) != objects.end()) {
        return false;
    }

    SceneObject obj;
    obj.id = id;
    obj.modelPath = modelPath;
    std::string local_path = modelPath;

    if (modelPath[0] == '@') {
        int s_pos = modelPath.find('/');
        if (s_pos > modelPath.find('\\')) {
            s_pos = modelPath.find('\\');
        }

        if (s_pos == std::string::npos) {
            return false;
        }

        std::string target = std::move(modelPath.substr(1, s_pos - 1));

        auto alias = std::find_if(path_aliases.begin(), path_aliases.end(),
            [&target](const std::pair<std::string, std::string>& p) {
                return p.first == target;
            });

        if (alias == path_aliases.end()) {
            return false;
        }

        local_path = alias->second + modelPath.substr(s_pos);
    }

    local_path = Utils::GetFullPath(local_path.c_str());

    if (!LoadModel(local_path, obj.instances)) {
        return false;
    }

    objects[id] = std::move(obj);
    return true;
}

bool Scene::RemoveObject(const std::string& id) {
    auto it = objects.find(id);
    if (it == objects.end()) {
        return false;
    }

    objects.erase(it);
    return true;
}

SceneObject* Scene::GetObject(const std::string& id) {
    auto it = objects.find(id);
    return (it != objects.end()) ? &it->second : nullptr;
}

const SceneObject* Scene::GetObject(const std::string& id) const {
    auto it = objects.find(id);
    return (it != objects.end()) ? &it->second : nullptr;
}

void Scene::SetObjectPosition(const std::string& id, const glm::vec3& position) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->position = position;
    }
}

void Scene::SetObjectRotation(const std::string& id, const glm::vec3& rotation) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->rotation = rotation;
    }
}

void Scene::SetObjectScale(const std::string& id, const glm::vec3& scale) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->scale = scale;
    }
}

void Scene::SetBGColor(float r, float g, float b) {
    bg_color.r = r / 255.0f;
    bg_color.g = g / 255.0f;
    bg_color.b = b / 255.0f;
}

void Scene::AddPathAlias(std::string key, std::string value) {
    path_aliases.emplace_back(key, value);
}

void Scene::MoveObject(const std::string& id, const glm::vec3& offset) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->position += offset;
    }
}

void Scene::RotateObject(const std::string& id, const glm::vec3& rotation) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->rotation += rotation;
    }
}

void Scene::ScaleObject(const std::string& id, const glm::vec3& scale) {
    SceneObject* obj = GetObject(id);
    if (obj) {
        obj->scale *= scale;
    }
}

void Scene::RenderScene(Renderer& renderer, const ICamera& camera) const {
    glClearColor(bg_color.r, bg_color.g, bg_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& [id, obj] : objects) {
        glm::mat4 objTransform = obj.GetTransform();
        renderer.RenderInstances(obj.instances, camera, objTransform);
    }
}

bool Scene::SaveToFile(const std::string& filePath) const {
    try {
        std::filesystem::path path(filePath);
        path = std::filesystem::absolute(path);

        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << path << std::endl;
            return false;
        }

        const char* signature = "SCENE001";
        file.write(signature, 8);

        file.write(reinterpret_cast<const char*>(&bg_color), sizeof(bg_color));

        size_t aliasCount = path_aliases.size();
        file.write(reinterpret_cast<const char*>(&aliasCount), sizeof(aliasCount));

        for (const auto& alias : path_aliases) {
            size_t keyLen = alias.first.length();
            file.write(reinterpret_cast<const char*>(&keyLen), sizeof(keyLen));
            file.write(alias.first.c_str(), keyLen);

            size_t valueLen = alias.second.length();
            file.write(reinterpret_cast<const char*>(&valueLen), sizeof(valueLen));
            file.write(alias.second.c_str(), valueLen);
        }

        size_t objectCount = objects.size();
        file.write(reinterpret_cast<const char*>(&objectCount), sizeof(objectCount));

        for (const auto& [id, obj] : objects) {
            obj.WriteToBinary(file);
        }

        file.close();
        std::cout << "Scene saved to: " << path << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving scene to file: " << e.what() << std::endl;
        return false;
    }
}

bool Scene::LoadFromFile(const std::string& filePath) {
    try {
        std::filesystem::path path(filePath);
        path = std::filesystem::absolute(path);


        if (!std::filesystem::exists(path)) {
            std::cerr << "File does not exist: " << path << std::endl;
            return false;
        }

        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for reading: " << path << std::endl;
            return false;
        }

        objects.clear();
        path_aliases.clear();

        char signature[9] = { 0 };
        if (!file.read(signature, 8)) {
            std::cerr << "Failed to read file signature" << std::endl;
            return false;
        }

        if (std::strncmp(signature, "SCENE001", 8) != 0) {
            std::cerr << "Invalid file format or version" << std::endl;
            return false;
        }

        if (!file.read(reinterpret_cast<char*>(&bg_color), sizeof(bg_color))) {
            std::cerr << "Failed to read background color" << std::endl;
            return false;
        }

        size_t aliasCount;
        if (!file.read(reinterpret_cast<char*>(&aliasCount), sizeof(aliasCount))) {
            std::cerr << "Failed to read alias count" << std::endl;
            return false;
        }

        if (aliasCount > 1000) {
            std::cerr << "Invalid alias count" << std::endl;
            return false;
        }

        for (size_t i = 0; i < aliasCount; ++i) {
            size_t keyLen, valueLen;

            if (!file.read(reinterpret_cast<char*>(&keyLen), sizeof(keyLen))) return false;
            if (keyLen > 1024) return false;

            std::string key(keyLen, '\0');
            if (!file.read(&key[0], keyLen)) return false;

            if (!file.read(reinterpret_cast<char*>(&valueLen), sizeof(valueLen))) return false;
            if (valueLen > 2048) return false;

            std::string value(valueLen, '\0');
            if (!file.read(&value[0], valueLen)) return false;

            path_aliases.emplace_back(key, value);
        }

        size_t objectCount;
        if (!file.read(reinterpret_cast<char*>(&objectCount), sizeof(objectCount))) {
            std::cerr << "Failed to read object count" << std::endl;
            return false;
        }

        if (objectCount > 10000) {
            std::cerr << "Invalid object count" << std::endl;
            return false;
        }

        for (size_t i = 0; i < objectCount; ++i) {
            SceneObject obj;
            if (!obj.ReadFromBinary(file)) {
                std::cerr << "Failed to read object " << i << std::endl;
                return false;
            }

            std::string local_path = obj.modelPath;

            if (obj.modelPath[0] == '@') {
                int s_pos = obj.modelPath.find('/');
                if (s_pos > obj.modelPath.find('\\')) {
                    s_pos = obj.modelPath.find('\\');
                }

                if (s_pos != std::string::npos) {
                    std::string target = obj.modelPath.substr(1, s_pos - 1);

                    auto alias = std::find_if(path_aliases.begin(), path_aliases.end(),
                        [&target](const std::pair<std::string, std::string>& p) {
                            return p.first == target;
                        });

                    if (alias != path_aliases.end()) {
                        local_path = alias->second + obj.modelPath.substr(s_pos);
                    }
                }
            }

            local_path = Utils::GetFullPath(local_path.c_str());

            if (LoadModel(local_path, obj.instances)) {
                objects[obj.id] = std::move(obj);
            }
            else {
                std::cerr << "Failed to load model for object: " << obj.id
                    << " at path: " << local_path << std::endl;
            }
        }

        file.close();
        std::cout << "Scene loaded from: " << path << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading scene from file: " << e.what() << std::endl;
        return false;
    }
}

bool Scene::LoadModel(const std::string& path, std::vector<ModelInstance>& instances) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool success = path.ends_with(".glb")
        ? loader.LoadBinaryFromFile(&model, &err, &warn, path)
        : loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!success) {
        return false;
    }

    for (const auto& node : model.nodes) {
        if (node.mesh < 0) continue;
        const auto& mesh = model.meshes[node.mesh];

        glm::mat4 transform = glm::mat4(1.0f);
        if (!node.matrix.empty()) {
            transform = glm::make_mat4x4(node.matrix.data());
        }
        else {
            glm::vec3 translation = node.translation.size() == 3 ?
                glm::vec3(static_cast<float>(node.translation[0]),
                    static_cast<float>(node.translation[1]),
                    static_cast<float>(node.translation[2])) : glm::vec3(0.0f);
            glm::vec3 scale = node.scale.size() == 3 ?
                glm::vec3(static_cast<float>(node.scale[0]),
                    static_cast<float>(node.scale[1]),
                    static_cast<float>(node.scale[2])) : glm::vec3(1.0f);
            glm::quat rotation = node.rotation.size() == 4 ?
                glm::quat(static_cast<float>(node.rotation[3]),
                    static_cast<float>(node.rotation[0]),
                    static_cast<float>(node.rotation[1]),
                    static_cast<float>(node.rotation[2])) : glm::quat();

            transform = glm::translate(glm::mat4(1.0f), translation) *
                glm::toMat4(rotation) *
                glm::scale(glm::mat4(1.0f), scale);
        }

        for (size_t primIndex = 0; primIndex < mesh.primitives.size(); ++primIndex) {
            const auto& prim = mesh.primitives[primIndex];

            std::string meshKey = path + "_mesh_" + std::to_string(node.mesh) + "_" + std::to_string(primIndex);

            MeshPrimitive* cachedMesh = ResourceManager::GetOrCreateMesh(meshKey, MeshPrimitive{});

            if (cachedMesh->vao == 0) {
                if (!LoadPrimitive(path, model, prim, *cachedMesh)) {
                    continue;
                }
            }

            ModelInstance instance;
            instance.mesh = cachedMesh;
            instance.transform = transform;
            instances.push_back(instance);
        }
    }

    return true;
}

bool Scene::LoadPrimitive(const std::string path, const tinygltf::Model& model, const tinygltf::Primitive& primitive, MeshPrimitive& meshPrim) {
    auto posIt = primitive.attributes.find("POSITION");
    if (posIt == primitive.attributes.end()) {
        return false;
    }

    const tinygltf::Accessor& posAccessor = model.accessors[posIt->second];
    const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
    const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];

    std::vector<float> normals;
    auto normalIt = primitive.attributes.find("NORMAL");
    if (normalIt != primitive.attributes.end()) {
        const tinygltf::Accessor& normalAccessor = model.accessors[normalIt->second];
        const tinygltf::BufferView& normalBufferView = model.bufferViews[normalAccessor.bufferView];
        const tinygltf::Buffer& normalBuffer = model.buffers[normalBufferView.buffer];

        const float* normalData = reinterpret_cast<const float*>(
            &normalBuffer.data[normalBufferView.byteOffset + normalAccessor.byteOffset]);
        normals.assign(normalData, normalData + normalAccessor.count * 3);
    }

    std::vector<float> uvs;
    auto uvIt = primitive.attributes.find("TEXCOORD_0");
    if (uvIt != primitive.attributes.end()) {
        const tinygltf::Accessor& uvAccessor = model.accessors[uvIt->second];
        const tinygltf::BufferView& uvBufferView = model.bufferViews[uvAccessor.bufferView];
        const tinygltf::Buffer& uvBuffer = model.buffers[uvBufferView.buffer];

        const float* uvData = reinterpret_cast<const float*>(
            &uvBuffer.data[uvBufferView.byteOffset + uvAccessor.byteOffset]);
        uvs.assign(uvData, uvData + uvAccessor.count * 2);
    }

    std::vector<unsigned int> indices;
    if (primitive.indices >= 0) {
        const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

        const unsigned char* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

        indices.resize(indexAccessor.count);
        if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
            const unsigned short* shortIndices = reinterpret_cast<const unsigned short*>(indexData);
            for (size_t i = 0; i < indexAccessor.count; ++i) {
                indices[i] = shortIndices[i];
            }
        }
        else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            const unsigned int* intIndices = reinterpret_cast<const unsigned int*>(indexData);
            indices.assign(intIndices, intIndices + indexAccessor.count);
        }

        meshPrim.indexCount = indices.size();
    }

    const float* posData = reinterpret_cast<const float*>(
        &posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset]);

    std::vector<float> vertices;
    size_t vertexCount = posAccessor.count;

    for (size_t i = 0; i < vertexCount; ++i) {
        vertices.push_back(posData[i * 3 + 0]);
        vertices.push_back(posData[i * 3 + 1]);
        vertices.push_back(posData[i * 3 + 2]);

        if (!normals.empty()) {
            vertices.push_back(normals[i * 3 + 0]);
            vertices.push_back(normals[i * 3 + 1]);
            vertices.push_back(normals[i * 3 + 2]);
        }
        else {
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }

        if (!uvs.empty()) {
            vertices.push_back(uvs[i * 2 + 0]);
            vertices.push_back(uvs[i * 2 + 1]);
        }
        else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    glGenVertexArrays(1, &meshPrim.vao);
    glGenBuffers(1, &meshPrim.vbo);
    glGenBuffers(1, &meshPrim.ebo);

    glBindVertexArray(meshPrim.vao);

    glBindBuffer(GL_ARRAY_BUFFER, meshPrim.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    if (!indices.empty()) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshPrim.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    if (primitive.material >= 0) {
        const tinygltf::Material& material = model.materials[primitive.material];

        if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
            const tinygltf::Texture& texture = model.textures[textureIndex];
            const tinygltf::Image& image = model.images[texture.source];

            std::string textureKey = path + "_texture_" + std::to_string(textureIndex);

            GLuint textureId = 0;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);

            GLenum format = GL_RGB;
            if (image.component == 4) {
                format = GL_RGBA;
            }
            else if (image.component == 1) {
                format = GL_RED;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0,
                format, GL_UNSIGNED_BYTE, image.image.data());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);

            meshPrim.texture = ResourceManager::GetOrCreateTexture(textureKey, textureId);
        }
    }

    return true;
}