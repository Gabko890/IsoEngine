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
#include "Renderer.hpp"

int main(int argc, char** argv) {
    Window window("ISO Engine Editor", 1920, 1080, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    EditorGUI gui(&window);

    SDL_GL_SetSwapInterval(-1);
    float dtime = 0;
    Uint64 ltime = SDL_GetTicks();

    SDL_HideCursor();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    Renderer renderer;
    if (!renderer.Initialize()) {
        SDL_Log("Failed to initialize renderer");
        return -1;
    }

    GLTFLoader loader;
    if (!loader.LoadModel(Utils::GetFullPath("../../assets/example_objects/test_cube.glb"))) {
        SDL_Log("Failed to load model");
        return -1;
    }

    SDL_Log("Loaded mesh instances: %zu", loader.GetInstances().size());

    FPSCamera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
    renderer.SetProjectionMatrix(projection);

    glm::vec3 modelPosition(0.0f);
    float anglex = 0.0f, angley = 0.0f, anglez = 0.0f;
    float modelScale = 1.0f;

    glm::vec3 lightPos(0.0f, 2.0f, 2.0f);
    glm::vec3 lightColor(1.0f);

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

        glm::mat4 modelTransform = glm::mat4(1.0f);
        modelTransform = glm::translate(modelTransform, modelPosition);
        modelTransform = glm::rotate(modelTransform, anglex, glm::vec3(1, 0, 0));
        modelTransform = glm::rotate(modelTransform, angley, glm::vec3(0, 1, 0));
        modelTransform = glm::rotate(modelTransform, anglez, glm::vec3(0, 0, 1));
        modelTransform = glm::scale(modelTransform, glm::vec3(modelScale));

        renderer.SetLightProperties(lightPos, lightColor);

        renderer.RenderInstances(loader.GetInstances(), camera, modelTransform);

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