#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <cstdlib>
#include <memory>
#include <vector>

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

const char* VERT_SRC = R"(#version 400
in vec3 a_position;
in vec4 a_color;
in vec2 a_uv;
out vec4 v_color;
out vec2 v_uv;

void main() {
    v_color = a_color;
    v_uv = a_uv;
    gl_Position = vec4(a_position, 1);
})";

const char* FRAG_SRC = R"(#version 400
uniform sampler2D f_tex;
in vec4 v_color;
in vec2 v_uv;

void main() {
    vec4 c = texture(f_tex, v_uv);
    gl_FragColor = v_color * c;
})";

template <class D>
class Handle final {
 private:
  Handle(const Handle&) = delete;
  Handle& operator=(const Handle&) = delete;

 private:
  GLuint id_ = 0;

 public:
  Handle() = default;
  Handle(GLuint id) : id_(id) {}
  Handle(Handle&& rhs) : id_(rhs.id_) { rhs.id_ = 0; }
  ~Handle() { D()(id_); }

  GLuint id() const { return id_; }
  explicit operator bool() const { return id_ != 0; }

  Handle& operator=(Handle&& rhs) {
    Handle(std::move(rhs)).swap(*this);
    return *this;
  }

  void swap(Handle& rhs) {
    using std::swap;
    swap(id_, rhs.id_);
  }
};

template <class D>
inline void swap(Handle<D>& lhs, Handle<D>& rhs) {
  lhs.swap(rhs);
}

struct DeleteShader {
  void operator()(GLuint id) const { glDeleteShader(id); }
};
using ShaderHandle = Handle<DeleteShader>;

struct DeleteProgram {
  void operator()(GLuint id) const { glDeleteProgram(id); }
};
using ProgramHandle = Handle<DeleteProgram>;

struct DeleteBuffer {
  void operator()(GLuint id) const { glDeleteBuffers(1, &id); }
};
using BufferHandle = Handle<DeleteBuffer>;

struct DeleteVertexArray {
  void operator()(GLuint id) const { glDeleteVertexArrays(1, &id); }
};
using VertexArrayHandle = Handle<DeleteVertexArray>;

struct DeleteTexture {
  void operator()(GLuint id) const { glDeleteTextures(1, &id); }
};
using TextureHandle = Handle<DeleteTexture>;

ShaderHandle compile_source(GLenum type, const char* src) {
  auto id = glCreateShader(type);
  if (id == 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "%s", gluErrorString(glGetError()));
    return 0;
  }

  const GLchar* srcs[] = {src};
  glShaderSource(id, 1, srcs, nullptr);
  glCompileShader(id);

  GLint status;
  glGetShaderiv(id, GL_COMPILE_STATUS, &status);
  if (status == GL_FALSE) {
    GLint len;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
      std::vector<char> buff(len);
      GLsizei size;
      glGetShaderInfoLog(id, len, &size, buff.data());
      SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "%s", buff.data());
    }
    return 0;
  }

  return id;
}

ProgramHandle create_program(const char* vert_src, const char* frag_src) {
  auto id = glCreateProgram();
  if (id == 0) {
    SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "%s", gluErrorString(glGetError()));
    return 0;
  }
  auto vert_shader = compile_source(GL_VERTEX_SHADER, vert_src);
  if (!vert_shader) {
    return 0;
  }
  auto frag_shader = compile_source(GL_FRAGMENT_SHADER, frag_src);
  if (!frag_shader) {
    return 0;
  }
  glAttachShader(id, vert_shader.id());
  glAttachShader(id, frag_shader.id());
  glLinkProgram(id);

  GLint status;
  glGetProgramiv(id, GL_LINK_STATUS, &status);
  if (status == GL_FALSE) {
    GLint len;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);
    if (len > 0) {
      std::vector<char> buff(len);
      GLsizei size;
      glGetProgramInfoLog(id, len, &size, buff.data());
      SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "%s", buff.data());
    }
    return 0;
  }

  return id;
}

BufferHandle create_buffer() {
  GLuint id;
  glGenBuffers(1, &id);
  return id;
}

VertexArrayHandle create_vertex_array() {
  GLuint id;
  glGenVertexArrays(1, &id);
  return id;
}

TextureHandle create_texture() {
  GLuint id;
  glGenTextures(1, &id);
  return id;
}

const std::uint32_t simple_texture[] = {
    0xff808080,
    0xffffffff,
    0xffffffff,
    0xff000000,
};

struct VertexFormat {
  float pos[3];
  float color[4];
  float uv[2];
};

// clang-format off
const VertexFormat vertices[] = {
    {
        { -0.5f, 0.5f, 0.0f, },
        { 1.0f, 1.0f, 1.0f, 1.0f, },
        { 0.0f, 0.0f, },
    },
    {
        { -0.5f, -0.5f, 0.0f, },
        { 1.0f, 0.0f, 0.0f, 1.0f, },
        { 0.0f, 1.0f, },
    },
    {
        { 0.5f, 0.5f, 0.0f, },
        { 0.0f, 1.0f, 0.0f, 1.0f, },
        { 1.0f, 0.0f, },
    },
    {
        { 0.5f, -0.5f, 0.0f, },
        { 0.0f, 0.0f, 0.0f, 1.0f, },
        { 1.0f, 1.0f, },
    },
};

const int indices[] = {
    0, 1, 2, 2, 1, 3,
};
// clang-format on

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

  glEnable(GL_TEXTURE_2D);

  auto shader = create_program(VERT_SRC, FRAG_SRC);
  if (!shader) {
    return EXIT_FAILURE;
  }

  auto vertex_array = create_vertex_array();
  glBindVertexArray(vertex_array.id());

  auto vertex_buffer = create_buffer();
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer.id());
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  auto index_buffer = create_buffer();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer.id());
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);

  auto texture = create_texture();
  glBindTexture(GL_TEXTURE_2D, texture.id());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               simple_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  auto pos_loc = glGetAttribLocation(shader.id(), "a_position");
  auto color_loc = glGetAttribLocation(shader.id(), "a_color");
  auto uv_loc = glGetAttribLocation(shader.id(), "a_uv");
  auto tex_loc = glGetAttribLocation(shader.id(), "f_tex");

  glEnableVertexAttribArray(pos_loc);
  glEnableVertexAttribArray(color_loc);
  glEnableVertexAttribArray(uv_loc);

  glVertexAttribPointer(
      pos_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat),
      reinterpret_cast<const void*>(offsetof(VertexFormat, pos)));
  glVertexAttribPointer(
      color_loc, 4, GL_FLOAT, GL_FALSE, sizeof(VertexFormat),
      reinterpret_cast<const void*>(offsetof(VertexFormat, color)));
  glVertexAttribPointer(
      uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat),
      reinterpret_cast<const void*>(offsetof(VertexFormat, uv)));

  glBindVertexArray(0);

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

    glUseProgram(shader.id());
    glBindVertexArray(vertex_array.id());
    glBindTexture(GL_TEXTURE_2D, texture.id());
    glUniform1i(tex_loc, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glFinish();
    SDL_GL_SwapWindow(window.get());
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  return EXIT_SUCCESS;
}
