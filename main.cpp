#include <SDL3/SDL.h>
#include <glad/glad.h>

#ifdef _EDITOR_BUILD
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#endif

#include <glm/vec3.hpp>

#include "Window.hpp"
#include "EditorGUI.hpp"

int main(int argc, char** argv) {
    Window window("ISO Engine Editor", 800, 600, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    EditorGUI gui(&window);

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

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        gui.Render_ImGui_Frame([]() {
            ImGui::ShowDemoWindow((bool*) 0);
        });

        window.Update();
    }
    return 0;
}