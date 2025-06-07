#include <SDL3/SDL.h>
#include <glad/glad.h>

#ifdef _EDITOR_BUILD
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.hpp"
#include "EditorGUI.hpp"
#include "GLTFLoader.hpp"

GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // Check for errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        SDL_Log("Shader compilation failed:\n%s", infoLog);
    }

    return shader;
}

GLuint CreateShaderProgram(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        SDL_Log("Shader linking failed:\n%s", infoLog);
    }

    // Delete shaders (they're linked now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

int main(int argc, char** argv) {
    Window window("ISO Engine Editor", 1920, 1080, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    EditorGUI gui(&window);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GLTFLoader loader;
    if (!loader.LoadModel("C:/Users/gabri/Downloads/monk_character.glb")) {
        SDL_Log("Failed to load model");
        return -1;
    }

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.7, 0.2, 1.0);
        }
    )";

    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    glm::mat4 projection = glm::perspective(45.0, 1920.0 / 1080.0, 0.1, 100.0);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, -1.5f, -4.0f));
    glm::mat4 modelMat = glm::mat4(1.0f);

    float anglex = 0.0f, angley = 0.0f, anglez = 0.0f;

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
#ifdef _EDITOR_BUILD
            ImGui_ImplSDL3_ProcessEvent(&event);
#endif
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        modelMat = glm::rotate(glm::mat4(1.0f), anglex, glm::vec3(1, 0, 0));
        modelMat = glm::rotate(modelMat, angley, glm::vec3(0, 1, 0));
        modelMat = glm::rotate(modelMat, anglez, glm::vec3(0, 0, 1));

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMat));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (const auto& prim : loader.GetPrimitives()) {
            glBindVertexArray(prim.vao);
            if (prim.ebo != 0)
                glDrawElements(GL_TRIANGLES, (GLsizei)prim.indexCount, GL_UNSIGNED_INT, nullptr);
            else
                glDrawArrays(GL_TRIANGLES, 0, (GLsizei)prim.indexCount);
        }
        glBindVertexArray(0);

        gui.Render_ImGui_Frame([&]() {
            ImGui::Begin("Rotation");
            ImGui::SliderAngle("X", &anglex, 0.f, 360.f);
            ImGui::SliderAngle("Y", &angley, 0.f, 360.f);
            ImGui::SliderAngle("Z", &anglez, 0.f, 360.f);
            ImGui::End();
        });

        window.Update();
    }

    return 0;
}