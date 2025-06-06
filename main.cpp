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
    Window window("ISO Engine Editor", 800, 600, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    EditorGUI gui(&window);

    bool running = true;
    SDL_Event event;

    float vertices[] = {
        // Triangle 1
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f,

         // Triangle 2
         -0.5f, -0.5f, 0.0f,
          0.5f,  0.5f, 0.0f,
         -0.5f,  0.5f, 0.0f
    };

    const char* vertexShaderSource = R"(
        #version 330 core

        layout (location = 0) in vec3 aPos;

        uniform mat4 model;

        void main() {
            gl_Position = model * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core

        out vec4 FragColor;

        void main() {
            FragColor = vec4(1.0, 0.7, 0.2, 1.0); // orange color
        }
    )";


    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Bind VAO first, then VBO
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Vertex attribute: location = 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    while (running) {
        while (SDL_PollEvent(&event)) {
            #ifdef _EDITOR_BUILD 
            ImGui_ImplSDL3_ProcessEvent(&event);
            #endif

            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }

        //glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT);

        static float anglex = 0;
        static float angley = 0;
        static float anglez = 0;

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        float time = (float)SDL_GetTicks() / 1000.0f;
        glm::mat4 modelx = glm::rotate(glm::mat4(1.0f), anglex, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 modely = glm::rotate(modelx, angley, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 modelz = glm::rotate(modely, anglez, glm::vec3(0.0f, 0.0f, 1.0f));
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");

        //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelz));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelz));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        
        gui.Render_ImGui_Frame([]() {
            //    ImGui::ShowDemoWindow((bool*) 0);
            ImGui::Begin("Rotation");
            ImGui::Text("Rotation:");
            ImGui::SliderAngle("x:", &anglex, 0.0f);
            ImGui::SliderAngle("y:", &angley, 0.0f);
            ImGui::SliderAngle("z:", &anglez, 0.0f);
            ImGui::End();
        });

        window.Update();
    }
    return 0;
}