#pragma once

#include "Shader.hpp"
#include "ModelInstance.hpp"
#include "Camera.hpp"
#include <glm/glm.hpp>
#include <vector>

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize();
    void SetProjectionMatrix(const glm::mat4& projection);
    void SetLightProperties(const glm::vec3& position, const glm::vec3& color);

    void RenderInstances(const std::vector<ModelInstance>& instances,
        const FPSCamera& camera,
        const glm::mat4& modelTransform = glm::mat4(1.0f));

private:
    Shader shader;
    glm::mat4 projectionMatrix;
    glm::vec3 lightPos;
    glm::vec3 lightColor;

    void BindTexture(GLuint textureID);

    static const char* GetVertexShaderSource();
    static const char* GetFragmentShaderSource();
};