#pragma once

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <iostream>

#ifdef _EDITOR_BUILD
#include <imgui.h>
#endif

class Window {
private:
	SDL_Window* sdl_window = NULL;
	SDL_GLContext gl_context = (SDL_GLContext) 0;


public: 
	Window(std::string title, int w, int h, SDL_WindowFlags flags);
	~Window();

	SDL_Window* Get_SDL_Window();
	SDL_GLContext Get_GL_Context();

	void Update();
};