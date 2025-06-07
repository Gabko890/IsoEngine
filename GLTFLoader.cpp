#include <tiny_gltf.h>
#include <glad/glad.h>
#include <vector>

#include "GLTFLoader.hpp"

GLTFLoader::GLTFLoader() {}

GLTFLoader::~GLTFLoader() {
    for (auto& prim : primitives) {
        glDeleteVertexArrays(1, &prim.vao);
        glDeleteBuffers(1, &prim.vbo);
        glDeleteBuffers(1, &prim.ebo);
    }
}

bool GLTFLoader::LoadModel(const std::string& path) {
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    bool ret = false;

    if (path.substr(path.size() - 4) == ".glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, path);
    }
    else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    }
    if (!ret) return false;

    for (const auto& mesh : model.meshes) {
        for (const auto& prim : mesh.primitives) {
            MeshPrimitive mp{};
            std::vector<float> vertices;
            std::vector<unsigned int> indices;

            {
                const tinygltf::Accessor& posAccessor = model.accessors[prim.attributes.at("POSITION")];
                const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];
                const tinygltf::Buffer& posBuffer = model.buffers[posView.buffer];
                const unsigned char* dataPtr = posBuffer.data.data() + posView.byteOffset + posAccessor.byteOffset;
                size_t count = posAccessor.count * 3;
                vertices.resize(count);
                memcpy(vertices.data(), dataPtr, count * sizeof(float));
            }

            if (prim.indices >= 0) {
                const tinygltf::Accessor& indexAccessor = model.accessors[prim.indices];
                const tinygltf::BufferView& indexView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer& indexBuffer = model.buffers[indexView.buffer];
                const unsigned char* dataPtr = indexBuffer.data.data() + indexView.byteOffset + indexAccessor.byteOffset;
                size_t count = indexAccessor.count;
                indices.resize(count);

                if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* src = reinterpret_cast<const uint16_t*>(dataPtr);
                    for (size_t i = 0; i < count; i++) indices[i] = src[i];
                }
                else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* src = reinterpret_cast<const uint32_t*>(dataPtr);
                    for (size_t i = 0; i < count; i++) indices[i] = src[i];
                }

                mp.indexCount = count;
            }
            else {
                mp.indexCount = vertices.size() / 3;
            }

            glGenVertexArrays(1, &mp.vao);
            glBindVertexArray(mp.vao);

            glGenBuffers(1, &mp.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, mp.vbo);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            if (!indices.empty()) {
                glGenBuffers(1, &mp.ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mp.ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            }

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

            glBindVertexArray(0);

            primitives.push_back(mp);
        }
    }

    return true;
}

const std::vector<MeshPrimitive>& GLTFLoader::GetPrimitives() const {
    return primitives;
}