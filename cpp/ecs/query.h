#pragma once
#include "chunk.h"
#include "entity.h"
#include "function_traits.h"

namespace ecs {

template <typename... Ts>
class Query {
 private:
  using query_type = type_list<Ts...>;

 private:
  Chunk* chunk_ = nullptr;

 public:
  explicit Query(Chunk* chunk) {
    for (auto it = chunk; it && !chunk_; it = it->next_chunk()) {
      if (it->contains<Ts...>()) {
        chunk_ = chunk;
      }
    }
  }

  template <typename F>
  void each(F f) {
    if (!chunk_) return;

    using args_type = typename function_traits<F>::args_type;
    for (auto chunk = chunk_; chunk; chunk = chunk_->next_chunk()) {
      chunk->each(f, args_type{});
    }
  }
  template <typename F, typename... Ts>
  void each(F f, type_list<Ts...> args) {
    for (auto chunk = chunk_; chunk; chunk = chunk_->next_chunk()) {
      chunk->each(f, args);
    }
  }
};

}  // namespace ecs
