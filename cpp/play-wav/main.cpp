#include <SDL.h>
#include <SDL_audio.h>

#include <cmath>
#include <cstdlib>
#include <memory>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"

namespace {

struct DestroyWindow {
  void operator()(SDL_Window* w) const { SDL_DestroyWindow(w); }
};
using WindowPtr = std::unique_ptr<SDL_Window, DestroyWindow>;

struct DestroyRenderer {
  void operator()(SDL_Renderer* r) const { SDL_DestroyRenderer(r); }
};
using RendererPtr = std::unique_ptr<SDL_Renderer, DestroyRenderer>;

class AudioDevice {
 private:
  AudioDevice(const AudioDevice&) = delete;
  AudioDevice(AudioDevice&&) = delete;
  AudioDevice& operator=(const AudioDevice&) = delete;
  AudioDevice& operator=(AudioDevice&&) = delete;

 private:
  SDL_AudioDeviceID device_ = 0;
  SDL_AudioSpec spec_;

 public:
  AudioDevice() = default;
  ~AudioDevice() { destroy(); }

  bool create() {
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = 44100;  // 44.1kHz
    spec.format = AUDIO_S16;
    spec.samples = 4096;

    return create_with_spec(spec);
  }
  bool create_with_spec(const SDL_AudioSpec& spec) {
    SDL_AudioSpec copy = spec;
    copy.callback = on_callback_static;
    copy.userdata = this;

    device_ = SDL_OpenAudioDevice(nullptr, 0, &copy, &spec_, 0);
    if (device_ == 0) {
      SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "%s", SDL_GetError());
      return false;
    }
    return true;
  }
  void destroy() {
    if (device_ > 0) {
      SDL_CloseAudioDevice(device_);
      device_ = 0;
    }
  }

 private:
  static void on_callback_static(void* arg, Uint8* stream, int len) {
    auto self = reinterpret_cast<AudioDevice*>(arg);
    SDL_memset(stream, self->spec_.silence, len);
    self->on_callback(stream, len);
  }

  float hz_ = 220.0f;
  float volume_ = 0.5f;
  int step_ = 0;

  float wave(float t) { return std::cosf(t * 2 * 3.14f); }

  void on_callback(Uint8* stream, int len) {
    auto frames = reinterpret_cast<Sint16*>(stream);
    auto channels = static_cast<int>(spec_.channels);
    auto size = len / (sizeof(Sint16) / sizeof(Uint8)) / channels;
    auto volume = SDL_MAX_SINT16 * volume_;
    for (int i = 0; i < size; ++i, ++step_) {
      for (int j = 0; j < channels; ++j) {
        frames[i * channels + j] = std::min<Sint16>(
            SDL_MAX_SINT16,
            std::max<Sint16>(
                SDL_MIN_SINT16,
                static_cast<Uint16>(wave(step_ * hz_ / spec_.freq) * volume)));
      }
    }
  }

 public:
  void on_debug_gui() {
    if (device_ == 0) return;

    ImGui::Text("freq: %d", spec_.freq);
    ImGui::Text("channels: %u", spec_.channels);
    ImGui::Text("samples: %u", spec_.samples);
    ImGui::Text("silence: %u", spec_.silence);
    ImGui::Text("size: %u", spec_.size);

    ImGui::Separator();

    ImGui::Text("status:");
    ImGui::SameLine();
    switch (SDL_GetAudioDeviceStatus(device_)) {
      case SDL_AUDIO_STOPPED:
        ImGui::Text("stopped");
        break;
      case SDL_AUDIO_PLAYING:
        ImGui::Text("playing");
        break;
      case SDL_AUDIO_PAUSED:
        ImGui::Text("paused");
        break;
      default:
        ImGui::Text("---");
        break;
    }

    ImGui::SliderFloat("Volume", &volume_, 0.0f, 1.0f);
    ImGui::InputFloat("Hz", &hz_);

    // clang-format off
    const struct {
      const char* name;
      float hz;
    } scale[] = {
        { "C", 130.813f, },
        { "D", 146.832f, },
        { "E", 164.814f, },
        { "F", 174.614f, },
        { "G", 195.998f, },
        { "A", 220.000f, },
        { "B", 246.942f, },
    };
    // clang-format on

    if (ImGui::Button("silence")) {
      hz_ = spec_.silence;
    }

    for (auto it : scale) {
      ImGui::SameLine();
      if (ImGui::Button(it.name)) {
        hz_ = it.hz;
      }
    }

    if (ImGui::Button("resume")) {
      SDL_PauseAudioDevice(device_, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("pause")) {
      SDL_PauseAudioDevice(device_, 1);
    }
  }
};

}  // namespace

int main(int /*argc*/, char* /*argv*/[]) {
  const char* TITLE = "Game";
  const int SCREEN_WIDTH = 16 * 40;
  const int SCREEN_HEIGHT = 9 * 40;

  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0) {
    return EXIT_FAILURE;
  }
  atexit(SDL_Quit);

  WindowPtr window(SDL_CreateWindow(TITLE, SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH,
                                    SCREEN_HEIGHT, 0));
  if (!window) {
    return EXIT_FAILURE;
  }

  RendererPtr renderer(SDL_CreateRenderer(
      window.get(), -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
  if (!renderer) {
    return EXIT_FAILURE;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
  ImGui_ImplSDLRenderer_Init(renderer.get());

  std::unique_ptr<AudioDevice> audio;
  int freq = 44100;  // 44.1kHz
  int channels = 2;
  int samples = 4096;  // ref. SDL_LoadWAV()

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

    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Audio");

    if (audio) {
      if (ImGui::Button("DESTROY")) {
        audio.reset();
      } else {
        audio->on_debug_gui();
      }
    } else {
      ImGui::InputInt("freq", &freq);
      ImGui::InputInt("channels", &channels);
      ImGui::InputInt("samples", &samples);
      if (ImGui::Button("CREATE")) {
        std::unique_ptr<AudioDevice> new_audio(new AudioDevice);
        SDL_AudioSpec spec;
        SDL_zero(spec);
        spec.freq = freq;
        spec.samples = static_cast<Uint16>(samples);
        spec.channels = static_cast<Uint8>(channels);
        spec.format = AUDIO_S16;
        if (new_audio->create_with_spec(spec)) {
          audio = std::move(new_audio);
        }
      }
    }

    ImGui::End();

    ImGui::Render();

    SDL_SetRenderDrawColor(renderer.get(), 0x12, 0x34, 0x56, 0x00);
    SDL_RenderClear(renderer.get());
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
    SDL_RenderPresent(renderer.get());
  }

  ImGui_ImplSDLRenderer_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  return EXIT_SUCCESS;
}
