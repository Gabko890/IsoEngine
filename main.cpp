#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>

#include <glad/glad.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Window.hpp"
#include "EditorGUI.hpp"
#include "GLTFLoader.hpp"
#include "Camera.hpp"
#include "Utils.hpp"

GLuint CompileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

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

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        SDL_Log("Shader linking failed:\n%s", infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

const char* vertexShaderSource = R"(
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

const char* fragmentShaderSource = R"(
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

int main(int argc, char** argv) {
    Window window("ISO Engine Editor", 1920, 1080, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    EditorGUI gui(&window);

    SDL_GL_SetSwapInterval(-1); // adaptive Vsync

    float dtime = 0;
    Uint64 ltime = SDL_GetTicks();

    SDL_HideCursor();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    GLTFLoader loader;
    if (!loader.LoadModel(Utils::GetFullPath("../../assets/example_objects/monk_character.glb"))) {
        SDL_Log("Failed to load model");
        return -1;
    }

    SDL_Log("Loaded mesh instances: %zu", loader.GetInstances().size());

    GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

    FPSCamera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);

    glm::vec3 modelPosition(0.0f);
    float anglex = 0.0f, angley = 0.0f, anglez = 0.0f;
    float modelScale = 1.0f;

    glm::vec3 lightPos(0.0f, 2.0f, 2.0f);
    glm::vec3 lightColor(1.0f);
    glm::vec3 lightRotation(0.0f);

    bool running = true;
    SDL_Event event;

    while (running) {
        Uint64 currentTime = SDL_GetTicks();
        dtime = (currentTime - ltime) / 1000.0f;
        ltime = currentTime;

        const Uint8* keystate = (Uint8*)SDL_GetKeyboardState(nullptr);
        if (keystate[SDL_SCANCODE_W]) camera.MoveForward(dtime, 5.0f);
        if (keystate[SDL_SCANCODE_S]) camera.MoveForward(dtime, -5.0f);
        if (keystate[SDL_SCANCODE_D]) camera.MoveRight(dtime, 5.0f);
        if (keystate[SDL_SCANCODE_A]) camera.MoveRight(dtime, -5.0f);

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            static bool move_toggle = false;
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT)
                move_toggle = true;
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_RIGHT)
                move_toggle = false;
            else if (event.type == SDL_EVENT_MOUSE_MOTION && move_toggle)
                camera.Rotate(-event.motion.yrel * 0.1f, event.motion.xrel * 0.1f);
        }

        glClearColor(130 / 255.0f, 200 / 255.0f, 229 / 255.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 guiTransform = glm::mat4(1.0f);
        guiTransform = glm::translate(guiTransform, modelPosition);
        guiTransform = glm::rotate(guiTransform, anglex, glm::vec3(1, 0, 0));
        guiTransform = glm::rotate(guiTransform, angley, glm::vec3(0, 1, 0));
        guiTransform = glm::rotate(guiTransform, anglez, glm::vec3(0, 0, 1));
        guiTransform = glm::scale(guiTransform, glm::vec3(modelScale));

        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

        for (const auto& inst : loader.GetInstances()) {
            glm::mat4 finalModel = guiTransform * inst.transform;
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(finalModel));

            if (inst.mesh->texture)
                glBindTexture(GL_TEXTURE_2D, inst.mesh->texture);

            glBindVertexArray(inst.mesh->vao);
            glDrawElements(GL_TRIANGLES, inst.mesh->indexCount, GL_UNSIGNED_INT, 0);
        }

        gui.Render_ImGui_Frame([&]() {
            ImGui::Begin("Transform");
            ImGui::SliderFloat3("Position", glm::value_ptr(modelPosition), -10.0f, 10.0f);
            ImGui::SliderAngle("Rotation X", &anglex);
            ImGui::SliderAngle("Rotation Y", &angley);
            ImGui::SliderAngle("Rotation Z", &anglez);
            ImGui::SliderFloat("Scale", &modelScale, 0.01f, 10.0f);
            ImGui::End();

            ImGui::Begin("Light");
            ImGui::SliderFloat3("Position", glm::value_ptr(lightPos), -10.0f, 10.0f);
            ImGui::ColorEdit3("Color", glm::value_ptr(lightColor));
            ImGui::End();
            });

        window.Update();
    }

    return 0;
}