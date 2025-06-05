#include "EditorGUI.hpp"
#include "Window.hpp"

#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>


EditorGUI::EditorGUI(Window* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();

    io->IniFilename = nullptr;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    EditorGUI::sdl_window = window;
    ImGui_ImplSDL3_InitForOpenGL(sdl_window->Get_SDL_Window(), sdl_window->Get_GL_Context());
    ImGui_ImplOpenGL3_Init("#version 330");
}

EditorGUI::~EditorGUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void EditorGUI::Render_ImGui_Frame(std::function<void()> Create_Frame) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    Create_Frame();

    ImGui::Render();
    glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}