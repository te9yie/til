#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <cstdlib>
#include <memory>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"

namespace {

struct DestroyWindow {
  void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
};
using WindowPtr = std::unique_ptr<SDL_Window, DestroyWindow>;

struct DeleteContext {
  void operator()(SDL_GLContext c) const { SDL_GL_DeleteContext(c); }
};
using ContextPtr =
    std::unique_ptr<std::remove_pointer_t<SDL_GLContext>, DeleteContext>;

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
  const char* TITLE = "Game";
  const int SCREEN_WIDTH = 16 * 80;
  const int SCREEN_HEIGHT = 9 * 80;

  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "%s", SDL_GetError());
    return EXIT_FAILURE;
  }
  atexit(SDL_Quit);

  WindowPtr window(SDL_CreateWindow(
      TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
      SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE));
  if (!window) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "%s", SDL_GetError());
    return EXIT_FAILURE;
  }

  ContextPtr context(SDL_GL_CreateContext(window.get()));
  if (!context) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "%s", SDL_GetError());
    return EXIT_FAILURE;
  }

  SDL_GL_MakeCurrent(window.get(), context.get());
  SDL_GL_SetSwapInterval(1);

  {
    auto err = glewInit();
    if (err != GLEW_OK) {
      SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "%s", glewGetErrorString(err));
      return EXIT_FAILURE;
    }
  }

  {
    int major;
    int minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    SDL_Log("GL Vendor: %s", glGetString(GL_VENDOR));
    SDL_Log("GL Renderer: %s", glGetString(GL_RENDERER));
    SDL_Log("GL Version: %s (%d.%d)", glGetString(GL_VERSION), major, minor);
    SDL_Log("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForOpenGL(window.get(), context.get());
  ImGui_ImplOpenGL3_Init("#version 400");

  bool loop = true;
  while (loop) {
    {
      SDL_Event e;
      while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT) {
          loop = false;
        }
      }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow(nullptr);

    ImGui::Render();

    int w, h;
    SDL_GL_GetDrawableSize(window.get(), &w, &h);
    glViewport(0, 0, w, h);

    glClearColor(0.12f, 0.34f, 0.56f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glFinish();
    SDL_GL_SwapWindow(window.get());
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  return EXIT_SUCCESS;
}
