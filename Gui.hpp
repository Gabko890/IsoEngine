#pragma once

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#include <functional>

#include "Window.hpp"

class Gui {
private:
	ImGuiIO* io = NULL;
    Window* sdl_window = NULL;
	bool new_frame = true;

public:
	Gui(Window* window);
	~Gui();

	void Add_GUI_Frame(std::function<void()> Create_Frame);
	void ApplyStyle(void);

	void Render(void);
};