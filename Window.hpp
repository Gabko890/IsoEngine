#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <Windows.h>
#include <iostream>

#ifdef _EDITOR_BUILD
#include <imgui.h>
#endif

class Window {
private:
	SDL_Window* window = NULL;
	SDL_GLContext gl_context = (SDL_GLContext) 0;

#ifdef _EDITOR_BUILD
	ImGuiIO* io = NULL;
#endif

public: 
	Window(std::string title, int w, int h, SDL_WindowFlags flags);
	~Window();

#ifdef _EDITOR_BUILD
	ImGuiIO& GetIO();
#endif

	void Destroy();
	void Update();
};