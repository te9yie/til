#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

using RawData = std::vector<float>;
using Data = std::vector<std::int16_t>;

RawData make_sin(int freq, float hz) {
  RawData data(freq);
  for (int i = 0; i < freq; ++i) {
    const float t = i * hz / freq;
    data[i] = std::sinf(t * 2 * 3.14159f);
  }
  return data;
}

struct ChunkHeader {
  char magic[4];
  std::uint32_t size;
};

struct RiffChunk {
  ChunkHeader header;
  char format[4];
};

struct FormatChunk {
  ChunkHeader header;
  std::uint16_t format;
  std::uint16_t channels;
  std::uint32_t freq;
  std::uint32_t byte_per_sec;
  std::uint16_t block_size;
  std::uint16_t samples;
};

void write_wav(std::string_view path, const Data& data) {
  const auto data_size = data.size() * sizeof(std::uint16_t);

  std::ofstream out(path.data(), std::ios::binary);

  ChunkHeader data_header;
  std::strncpy(data_header.magic, "data", sizeof(data_header.magic));
  data_header.size = static_cast<std::int32_t>(data_size);

  FormatChunk format;
  std::strncpy(format.header.magic, "fmt ", sizeof(format.header.magic));
  format.header.size = sizeof(format) - sizeof(format.header);
  format.format = 1;
  format.channels = 1;
  format.freq = 44100;
  format.block_size = 2;
  format.samples = 16;
  format.byte_per_sec = format.freq * format.channels * format.block_size;

  RiffChunk riff;
  std::strncpy(riff.header.magic, "RIFF", sizeof(riff.header.magic));
  riff.header.size = static_cast<std::uint32_t>(
      sizeof(riff) - sizeof(riff.header) + sizeof(format) +
      sizeof(data_header) + data_size);
  std::strncpy(riff.format, "WAVE", sizeof(riff.format));

  out.write(reinterpret_cast<const char*>(&riff), sizeof(riff));
  out.write(reinterpret_cast<const char*>(&format), sizeof(format));
  out.write(reinterpret_cast<const char*>(&data_header), sizeof(data_header));
  out.write(reinterpret_cast<const char*>(data.data()), data_size);
}

int main() {
  const int freq = 44100;

  RawData raw = make_sin(freq, 220.0f);

  Data data(raw.size());
  std::transform(raw.begin(), raw.end(), data.begin(), [](float v) {
    constexpr int volume =
        std::numeric_limits<std::int16_t>::max() * 8 / 10;  // 80%
    return static_cast<std::int16_t>(v * volume);
  });

  write_wav("out.wav", data);

  return 0;
}
