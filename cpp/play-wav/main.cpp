#include <SDL.h>
#include <SDL_audio.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <vector>

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

std::vector<float> make_saw(int freq, float hz) {
  std::vector<float> data(freq);
  for (int i = 0; i < freq; ++i) {
    const float t = i * hz / freq;
    const float ft = t - std::floorf(t);
    data[i] = (ft - 0.5f) * 2.0f;
  }
  return data;
}
std::vector<float> make_sin(int freq, float hz) {
  std::vector<float> data(freq);
  for (int i = 0; i < freq; ++i) {
    const float t = i * hz / freq;
    data[i] = std::sinf(t * 2 * 3.14159f);
  }
  return data;
}

std::vector<float> make_rosenberg(int freq, float hz, float tau1, float tau2) {
  std::vector<float> data(freq);
  for (int i = 0; i < freq; ++i) {
    const float t = i * hz / freq;
    const float ft = t - std::floorf(t);
    if (ft <= tau1) {
      data[i] =
          3.0f * std::powf(ft / tau1, 2.0f) - 2.0f * std::powf(ft / tau1, 3.0f);
    } else if (ft < tau1 + tau2) {
      data[i] = 1.0f - std::powf((ft - tau1) / tau2, 2.0f);
    } else {
      data[i] = 0.0f;
    }
  }
  return data;
}

class SoundPlayer {
 private:
  const float* data_ptr_ = nullptr;
  std::size_t data_size_ = 0;
  std::size_t play_index_ = 0;

 public:
  SoundPlayer() = default;

  void setup(const float* data, std::size_t size) {
    data_ptr_ = data;
    data_size_ = size;
    play_index_ = 0;
  }
  void tear_down() {
    data_ptr_ = nullptr;
    data_size_ = 0;
    play_index_ = 0;
  }

  float next() {
    if (play_index_ < data_size_) {
      return *(data_ptr_ + play_index_++);
    }
    return 0.0f;
  }

  void rewind() { play_index_ = 0; }
};

class AudioDevice {
 private:
  AudioDevice(const AudioDevice&) = delete;
  AudioDevice(AudioDevice&&) = delete;
  AudioDevice& operator=(const AudioDevice&) = delete;
  AudioDevice& operator=(AudioDevice&&) = delete;

 private:
  SDL_AudioDeviceID device_ = 0;
  SDL_AudioSpec spec_;
  float volume_ = 0.5f;

  std::vector<float> sound_data_;
  SoundPlayer player_;

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

  void on_callback(Uint8* stream, int len) {
    auto frames = reinterpret_cast<Sint16*>(stream);
    auto channels = static_cast<int>(spec_.channels);
    auto size = len / (sizeof(Sint16) / sizeof(Uint8)) / channels;
    auto volume = 30000 /* nearly SDL_MAX_SINT16 */ * volume_;
    for (int i = 0; i < size; ++i) {
      const auto p = player_.next();
      for (int j = 0; j < channels; ++j) {
        frames[i * channels + j] = static_cast<Sint16>(std::clamp<int>(
            static_cast<int>(p * volume), SDL_MIN_SINT16, SDL_MAX_SINT16));
      }
    }
  }

 public:
  enum {
    SIN,
    SAW,
    ROSENBERG,
  };

  int type_ = SIN;
  float tau1_ = 0.9f;
  float tau2_ = 0.05f;

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

    if (ImGui::Button("resume")) {
      SDL_PauseAudioDevice(device_, 0);
    }
    ImGui::SameLine();
    if (ImGui::Button("pause")) {
      SDL_PauseAudioDevice(device_, 1);
    }

    const char* TYPE_NAME[] = {"Sin", "Saw", "Rosenberg"};
    ImGui::Combo("type", &type_, TYPE_NAME,
                 static_cast<int>(std::size(TYPE_NAME)));
    switch (type_) {
      case ROSENBERG:
        ImGui::InputFloat("tau1", &tau1_);
        ImGui::InputFloat("tau2", &tau2_);
        break;
    }
    ImGui::SliderFloat("Volume", &volume_, 0.0f, 1.0f);

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

    ImGui::Spacing();
    for (auto it : scale) {
      ImGui::SameLine();
      if (ImGui::Button(it.name)) {
        SDL_LockAudioDevice(device_);
        switch (type_) {
          case SIN:
            sound_data_ = make_sin(spec_.freq, it.hz);
            break;
          case SAW:
            sound_data_ = make_saw(spec_.freq, it.hz);
            break;
          case ROSENBERG:
            sound_data_ = make_rosenberg(spec_.freq, it.hz, tau1_, tau2_);
            break;
        }
        player_.setup(sound_data_.data(), sound_data_.size());
        SDL_UnlockAudioDevice(device_);
      }
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
