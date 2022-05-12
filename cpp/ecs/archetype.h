#pragma once
#include "chunk.h"
#include "tuple.h"

namespace ecs {

class Archetype {
 private:
  Archetype(const Archetype&) = delete;
  Archetype(Archetype&&) = delete;
  Archetype& operator=(const Archetype&) = delete;
  Archetype& operator=(Archetype&&) = delete;

 private:
  std::unique_ptr<Tuple> tuple_;
  Chunk* for_iter_ = nullptr;

 public:
  Archetype(std::unique_ptr<Tuple>&& tuple) : tuple_(std::move(tuple)) {}

  Chunk* get_free_chunk() {
    for (Chunk* chunk = for_iter_; chunk;
         chunk = chunk->next_same_archetype_chunk()) {
      if (!chunk->is_full()) return chunk;
    }
    return nullptr;
  }

  void link_chunk(Chunk* chunk) {
    for (auto it = for_iter_; it; it = it->next_same_archetype_chunk()) {
      if (it->next_same_archetype_chunk()) continue;
      it->link_same_archetype_chunk(chunk);
      return;
    }
    for_iter_ = chunk;
  }

  const Tuple* tuple() const { return tuple_.get(); }

  template <typename... Ts>
  bool is_match() const {
    return tuple_->is_match<Ts...>();
  }

 public:
  template <typename... Ts>
  static std::unique_ptr<Archetype> make() {
    auto tuple = Tuple::make<Ts...>();
    return std::unique_ptr<Archetype>(new Archetype(std::move(tuple)));
  }
};

}  // namespace ecs
