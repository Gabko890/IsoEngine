#include <SDL3/SDL.h>

#ifdef _EDITOR_BUILD
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_opengl3.h>
#endif

#include "Window.hpp"


/*
 * Constructors / deconstructors
 */

Window::Window(std::string title, int w, int h, SDL_WindowFlags flags)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        throw std::runtime_error("Failed to initialize SDL");
    }

    SDL_SetHint(SDL_HINT_OPENGL_ES_DRIVER, "0"); // Desktop OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    sdl_window = SDL_CreateWindow(title.c_str(), w, h, SDL_WINDOW_OPENGL | flags);
    if (!sdl_window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("Failed to create window");
    }

    gl_context = SDL_GL_CreateContext(sdl_window);
    if (!gl_context) {
        SDL_Log("Failed to create OpenGL context: %s", SDL_GetError());
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        throw std::runtime_error("Failed to create OpenGL context");
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        SDL_Log("Failed to initialize OpenGL context");
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        throw std::runtime_error("Failed to initialize OpenGL context");
    }

    w_active = true;
}

Window::~Window() {
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}


/*
 * Setters / getters
 */

SDL_Window* Window::Get_SDL_Window() {
    return sdl_window;
}

SDL_GLContext Window::Get_GL_Context() {
    return gl_context;
}


/*
 * Window managment
 */

void Window::Update() {
    SDL_GL_SwapWindow(sdl_window);
}