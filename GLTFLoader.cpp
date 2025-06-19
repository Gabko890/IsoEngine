#include "GLTFLoader.hpp"
#include "ResourceManager.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glad/glad.h>

#include <string>
#include <string_view>
#include <vector>

bool GLTFLoader::LoadModel(const std::string& path) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool success = path.ends_with(".glb")
        ? loader.LoadBinaryFromFile(&model, &err, &warn, path)
        : loader.LoadASCIIFromFile(&model, &err, &warn, path);

    if (!success) {
        if (!err.empty()) {
            printf("GLTF Error: %s\n", err.c_str());
        }
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

bool GLTFLoader::LoadPrimitive(const std::string path, const tinygltf::Model& model, const tinygltf::Primitive& primitive, MeshPrimitive& meshPrim) {
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

const std::vector<ModelInstance>& GLTFLoader::GetInstances() const {
    return instances;
}