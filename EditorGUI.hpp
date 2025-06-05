#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <functional>

#include "Window.hpp"

class EditorGUI {
private:
	ImGuiIO* io = NULL;
    Window* sdl_window = NULL;

public:
	EditorGUI(Window* window);
	~EditorGUI();

	void Render_ImGui_Frame(std::function<void()> Create_Frame);
};