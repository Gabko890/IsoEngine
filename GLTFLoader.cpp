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
        if (prim.texture) glDeleteTextures(1, &prim.texture);
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
            std::vector<float> buffer;
            std::vector<unsigned int> indices;

            const auto& posAcc = model.accessors[prim.attributes.at("POSITION")];
            const auto& normAcc = model.accessors[prim.attributes.at("NORMAL")];
            const auto& uvAcc = prim.attributes.count("TEXCOORD_0") ? model.accessors[prim.attributes.at("TEXCOORD_0")] : tinygltf::Accessor();

            const auto& posView = model.bufferViews[posAcc.bufferView];
            const auto& posBuf = model.buffers[posView.buffer];
            const float* posData = reinterpret_cast<const float*>(&posBuf.data[posView.byteOffset + posAcc.byteOffset]);

            const auto& normView = model.bufferViews[normAcc.bufferView];
            const auto& normBuf = model.buffers[normView.buffer];
            const float* normData = reinterpret_cast<const float*>(&normBuf.data[normView.byteOffset + normAcc.byteOffset]);

            const float* uvData = nullptr;
            if (prim.attributes.count("TEXCOORD_0")) {
                const auto& uvView = model.bufferViews[uvAcc.bufferView];
                const auto& uvBuf = model.buffers[uvView.buffer];
                uvData = reinterpret_cast<const float*>(&uvBuf.data[uvView.byteOffset + uvAcc.byteOffset]);
            }

            for (size_t i = 0; i < posAcc.count; i++) {
                buffer.push_back(posData[i * 3 + 0]);
                buffer.push_back(posData[i * 3 + 1]);
                buffer.push_back(posData[i * 3 + 2]);

                buffer.push_back(normData[i * 3 + 0]);
                buffer.push_back(normData[i * 3 + 1]);
                buffer.push_back(normData[i * 3 + 2]);

                if (uvData) {
                    buffer.push_back(uvData[i * 2 + 0]);
                    buffer.push_back(uvData[i * 2 + 1]);
                }
                else {
                    buffer.push_back(0.0f);
                    buffer.push_back(0.0f);
                }
            }

            if (prim.indices >= 0) {
                const auto& idxAcc = model.accessors[prim.indices];
                const auto& idxView = model.bufferViews[idxAcc.bufferView];
                const auto& idxBuf = model.buffers[idxView.buffer];
                const unsigned char* idxData = idxBuf.data.data() + idxView.byteOffset + idxAcc.byteOffset;
                size_t count = idxAcc.count;
                indices.resize(count);

                if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                    const uint16_t* src = reinterpret_cast<const uint16_t*>(idxData);
                    for (size_t i = 0; i < count; ++i) indices[i] = src[i];
                }
                else if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                    const uint32_t* src = reinterpret_cast<const uint32_t*>(idxData);
                    for (size_t i = 0; i < count; ++i) indices[i] = src[i];
                }

                mp.indexCount = count;
            }
            else {
                mp.indexCount = buffer.size() / 8;
            }

            glGenVertexArrays(1, &mp.vao);
            glBindVertexArray(mp.vao);

            glGenBuffers(1, &mp.vbo);
            glBindBuffer(GL_ARRAY_BUFFER, mp.vbo);
            glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);

            if (!indices.empty()) {
                glGenBuffers(1, &mp.ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mp.ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            }

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));

            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

            glBindVertexArray(0);

            if (prim.material >= 0) {
                const auto& mat = model.materials[prim.material];
                if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                    const auto& tex = model.textures[mat.pbrMetallicRoughness.baseColorTexture.index];
                    const auto& img = model.images[tex.source];

                    glGenTextures(1, &mp.texture);
                    glBindTexture(GL_TEXTURE_2D, mp.texture);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.width, img.height, 0, img.component == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, img.image.data());
                    glGenerateMipmap(GL_TEXTURE_2D);

                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                }
            }

            primitives.push_back(mp);
        }
    }

    return true;
}

const std::vector<MeshPrimitive>& GLTFLoader::GetPrimitives() const {
    return primitives;
}