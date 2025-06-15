#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

class Shader {
public:
    Shader() = default;
    ~Shader();

    bool CreateFromSource(const char* vertexSource, const char* fragmentSource);
    bool CreateFromFiles(const std::string& vertexPath, const std::string& fragmentPath);

    void Use() const;
    void Delete();

    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec3(const std::string& name, const glm::vec3& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat4(const std::string& name, const glm::mat4& mat) const;

    GLuint GetID() const { return programID; }

private:
    GLuint programID = 0;

    GLuint CompileShader(GLenum type, const char* source);
    void CheckShaderCompilation(GLuint shader, const std::string& type);
    void CheckProgramLinking(GLuint program);
};