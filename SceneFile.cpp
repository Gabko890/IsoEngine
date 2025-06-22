#include "Scene.hpp"
#include "Utils.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>

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

void SceneObject::WriteToBinary(std::ofstream& file) const {
    size_t idLen = id.length();
    file.write(reinterpret_cast<const char*>(&idLen), sizeof(idLen));
    file.write(id.c_str(), idLen);

    size_t pathLen = modelPath.length();
    file.write(reinterpret_cast<const char*>(&pathLen), sizeof(pathLen));
    file.write(modelPath.c_str(), pathLen);

    file.write(reinterpret_cast<const char*>(&position), sizeof(position));
    file.write(reinterpret_cast<const char*>(&rotation), sizeof(rotation));
    file.write(reinterpret_cast<const char*>(&scale), sizeof(scale));
}

bool SceneObject::ReadFromBinary(std::ifstream& file) {
    try {
        size_t idLen;
        if (!file.read(reinterpret_cast<char*>(&idLen), sizeof(idLen))) return false;
        if (idLen > 1024) return false;

        id.resize(idLen);
        if (!file.read(&id[0], idLen)) return false;

        size_t pathLen;
        if (!file.read(reinterpret_cast<char*>(&pathLen), sizeof(pathLen))) return false;
        if (pathLen > 2048) return false;

        modelPath.resize(pathLen);
        if (!file.read(&modelPath[0], pathLen)) return false;

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