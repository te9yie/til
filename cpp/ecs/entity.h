#pragma once
#include <cstddef>

namespace ecs {

class Chunk;

struct EntityId {
  std::size_t generation = 0;
  std::size_t index = 0;
};

struct EntityStorage {
  std::size_t generation = 0;
  Chunk* chunk = nullptr;
  size_t chunk_index = 0;
};

}  // namespace ecs
