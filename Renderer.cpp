#include "Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

Renderer::Renderer()
    : projectionMatrix(1.0f)
    , lightPos(0.0f, 2.0f, 2.0f)
    , lightColor(1.0f, 1.0f, 1.0f)
{
}

Renderer::~Renderer() {
    shader.Delete();
}

bool Renderer::Initialize() {
    return shader.CreateFromSource(GetVertexShaderSource(), GetFragmentShaderSource());
}

void Renderer::SetProjectionMatrix(const glm::mat4& projection) {
    projectionMatrix = projection;
}

void Renderer::SetLightProperties(const glm::vec3& position, const glm::vec3& color) {
    lightPos = position;
    lightColor = color;
}

void Renderer::RenderInstances(const std::vector<ModelInstance>& instances,
    const ICamera& camera,
    const glm::mat4& modelTransform) {
    shader.Use();

    shader.SetMat4("view", camera.GetViewMatrix());
    shader.SetMat4("projection", projectionMatrix);
    shader.SetVec3("lightPos", lightPos);
    shader.SetVec3("lightColor", lightColor);
    shader.SetInt("baseColorTexture", 0);

    for (const auto& instance : instances) {
        if (!instance.mesh || instance.mesh->vao == 0) continue;

        glm::mat4 finalModel = modelTransform * instance.transform;
        shader.SetMat4("model", finalModel);

        BindTexture(instance.mesh->texture);

        glBindVertexArray(instance.mesh->vao);
        if (instance.mesh->indexCount > 0) {
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(instance.mesh->indexCount), GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
    }
}

void Renderer::BindTexture(GLuint textureID) {
    glActiveTexture(GL_TEXTURE0);
    if (textureID != 0) {
        glBindTexture(GL_TEXTURE_2D, textureID);
    }
    else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

const char* Renderer::GetVertexShaderSource() {
    return R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aUV;

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoord;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            TexCoord = aUV;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";
}

const char* Renderer::GetFragmentShaderSource() {
    return R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoord;

        out vec4 FragColor;

        uniform sampler2D baseColorTexture;
        uniform vec3 lightPos;
        uniform vec3 lightColor;

        void main() {
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 light = diff * lightColor;

            vec4 texColor = texture(baseColorTexture, TexCoord);
            FragColor = vec4(texColor.rgb * light, texColor.a);
        }
    )";
}