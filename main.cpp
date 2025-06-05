#include <SDL3/SDL.h>
#include <glad/glad.h>

#ifdef _EDITOR_BUILD
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#endif

#include "Window.hpp"

int main(int argc, char** argv) {
    Window window("SDL3 w openGL", 800, 600, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);

    // Main loop
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

        #ifdef _EDITOR_BUILD
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // Create a sample ImGui window
        static bool show_demo_window = true;
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        // Custom window example
        ImGui::Begin("Hello, ImGui!");
        ImGui::Text("This is some useful text.");
        if (ImGui::Button("Close Demo")) {
            show_demo_window = false;
        }
        ImGui::End();

        // Rendering
        ImGui::Render();
        
        glViewport(0, 0, (int)window.GetIO().DisplaySize.x, (int)window.GetIO().DisplaySize.y);
        #endif
        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        #ifdef _EDITOR_BUILD 
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        #endif
        
        //SDL_GL_SwapWindow(window);
        window.Update();
    }
    return 0;
}