#pragma once
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace t9 {

constexpr std::uint32_t FNV_OFFSET_BASIS = 2166136261u;
constexpr std::uint32_t FNV_PRIME = 16777619u;

inline void fnv_1a(std::uint32_t* hash, const void* buf, std::size_t n) {
  auto p = reinterpret_cast<const std::uint8_t*>(buf);
  while (n--) {
    *hash = (*hash ^ *p++) * FNV_PRIME;
  }
}

inline std::uint32_t make_string_hash(std::string_view str) {
  std::uint32_t hash = FNV_OFFSET_BASIS;
  fnv_1a(&hash, str.data(), str.length());
  return hash;
}

}  // namespace t9