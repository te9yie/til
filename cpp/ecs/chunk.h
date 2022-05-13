#pragma once
#include <cassert>
#include <cstdint>
#include <vector>

#include "function_traits.h"
#include "tuple.h"

namespace ecs {

class Chunk {
 private:
  Chunk(const Chunk&) = delete;
  Chunk(Chunk&&) = delete;
  Chunk& operator=(const Chunk&) = delete;
  Chunk& operator=(Chunk&&) = delete;

 public:
  template <typename... Ts>
  struct AccessTuple {
    using tuple_type = std::tuple<Ts...>;
    template <std::size_t I>
    using element_type = std::tuple_element_t<I, tuple_type>;

    Chunk* chunk = nullptr;
    std::size_t index = 0;

    AccessTuple() = default;
    AccessTuple(Chunk* chunk, std::size_t index) : chunk(chunk), index(index) {}

    template <std::size_t I>
    element_type<I> get() {
      return *chunk->get<element_type<I>>(index);
    }
  };

 private:
  static const std::size_t BUFF_SIZE = 16 * 1024;

  struct IndexDesc {
    std::size_t next = 0;
    bool use = false;
  };

 private:
  const Tuple* tuple_ = nullptr;
  std::vector<IndexDesc> indices_;
  std::size_t next_index_ = 0;
  std::size_t peak_index_ = 0;
  Chunk* next_chunk_ = nullptr;
  Chunk* next_same_archetype_chunk_ = nullptr;
  std::uint8_t buff_[BUFF_SIZE] = {0};

 public:
  Chunk(const Tuple* tuple)
      : tuple_(tuple), indices_(BUFF_SIZE / tuple->memory_size()) {}

  std::size_t create() {
    assert(next_index_ < capacity());
    if (next_index_ >= peak_index_) {
      indices_[peak_index_].next = peak_index_ + 1;
      ++peak_index_;
    }
    auto index = next_index_;
    indices_[next_index_].use = true;
    next_index_ = indices_[next_index_].next;

    auto ptr = buff_ + tuple_->memory_size() * index;
    tuple_->construct(ptr);
    return index;
  }
  void destroy(std::size_t index) {
    auto ptr = buff_ + tuple_->memory_size() * index;
    tuple_->destruct(ptr);

    indices_[index].use = false;
    indices_[index].next = next_index_;
    next_index_ = index;
  }

  template <typename T>
  T* get(std::size_t index) {
    std::size_t offset = 0;
    if (!tuple_->try_get_offset<T>(&offset)) return nullptr;
    auto ptr = buff_ + tuple_->memory_size() * index + offset;
    return reinterpret_cast<T*>(ptr);
  }

  template <typename... Ts>
  AccessTuple<Ts...> as_tuple(std::size_t index) {
    return AccessTuple<Ts...>(this, index);
  }

  template <typename F, typename... Ts>
  void each(F f, type_list<Ts...>) {
    for (std::size_t i = 0; i < peak_index_; ++i) {
      if (!indices_[i].use) continue;
      f(*get<sanitalize_t<Ts>>(i)...);
    }
  }

  std::size_t capacity() const { return indices_.size(); }
  std::size_t size() const { return peak_index_; }
  bool is_full() const { return next_index_ >= indices_.size(); }

  template <typename... Ts>
  bool contains() {
    return tuple_->contains<Ts...>();
  }

  Chunk* next_chunk() { return next_chunk_; }
  Chunk* next_same_archetype_chunk() { return next_same_archetype_chunk_; }

  void link_chunk(Chunk* chunk) { next_chunk_ = chunk; }
  void link_same_archetype_chunk(Chunk* chunk) {
    next_same_archetype_chunk_ = chunk;
  }
};

}  // namespace ecs
