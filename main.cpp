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
#include "Scene.hpp"
#include "Camera.hpp"
#include "Utils.hpp"
#include "Renderer.hpp"

#include "Editor.hpp"
#include "TerminalHelper.hpp"


int main(int argc, char** argv) {
    Window window("ISO Engine Editor", 1920, 1080, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    Editor editor(&window);
    editor.ApplyStyle();

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

    Scene scene;
    TerminalHelper::scene = &scene;

    SDL_Log("Scene loaded successfully");

    FPSCamera camera(glm::vec3(0.0f, 0.0f, 5.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1920.0f / 1080.0f, 0.1f, 100.0f);
    renderer.SetProjectionMatrix(projection);

    glm::vec3 lightPos(0.0f, 2.0f, 2.0f);
    glm::vec3 lightColor(1.0f);

    TerminalHelper terminal_helper;
    ImTerm::terminal<TerminalHelper> term;

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

        editor.Add_GUI_Frame([&]() {
            ImGui::Begin("Scene Objects");
            for (const auto& [id, obj] : scene.GetObjects()) {
                if (ImGui::TreeNode(id.c_str())) {
                    ImGui::Text("Model: %s", obj.modelPath.c_str());

                    glm::vec3 pos = obj.position;
                    if (ImGui::SliderFloat3("Position", glm::value_ptr(pos), -10.0f, 10.0f)) {
                        scene.SetObjectPosition(id, pos);
                    }

                    glm::vec3 rot = obj.rotation;
                    if (ImGui::SliderFloat3("Rotation", glm::value_ptr(rot), -3.14159f, 3.14159f)) {
                        scene.SetObjectRotation(id, rot);
                    }

                    glm::vec3 scale = obj.scale;
                    if (ImGui::SliderFloat3("Scale", glm::value_ptr(scale), 0.01f, 10.0f)) {
                        scene.SetObjectScale(id, scale);
                    }

                    ImGui::TreePop();
                }
            }
            ImGui::End();

            ImGui::Begin("Light");
            ImGui::SliderFloat3("Position", glm::value_ptr(lightPos), -10.0f, 10.0f);
            ImGui::ColorEdit3("Color", glm::value_ptr(lightColor));
            ImGui::End();


            ImGui::SetNextWindowPos(ImVec2(0, 1080 - 250), ImGuiCond_Always);
            ImGui::SetNextWindowSizeConstraints(ImVec2(1920, 100), ImVec2(1920, 600));
            ImGui::SetNextWindowSize(ImVec2(1920, 200), ImGuiCond_Always);
            term.show();

            ImGui::SetNextWindowPos(ImVec2(0, 1080 - 50), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(1920, 50), ImGuiCond_Always);
            ImGui::Begin(" ", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
            ImGui::Text(" ");
            ImGui::Dummy(ImGui::GetContentRegionAvail());
            ImGui::End();
        });
        

        renderer.SetLightProperties(lightPos, lightColor);
        scene.RenderScene(renderer, camera);

        editor.Render();

        window.Update();
    }

    return 0;
}