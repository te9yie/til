#pragma once
#include "chunk.h"
#include "entity.h"
#include "function_traits.h"

namespace ecs {

template <typename... Ts>
class QueryIterator {
 private:
  using Tuple = Chunk::AccessTuple<Ts...>;

 private:
  Chunk* chunk_ = nullptr;
  std::size_t chunk_index_ = 0;
  Tuple tuple_;

 public:
  QueryIterator() = default;
  QueryIterator(Chunk* chunk) : chunk_(chunk), chunk_index_(0) {
    check_index();
  }

  QueryIterator& operator++() {
    ++chunk_index_;
    check_index();
    return *this;
  }

  Tuple* operator->() { return &tuple_; }
  Tuple& operator*() { return tuple_; }

  bool operator==(const QueryIterator& rhs) const {
    return chunk_ == rhs.chunk_ && chunk_index_ == rhs.chunk_index_;
  }
  bool operator!=(const QueryIterator& rhs) const { return !operator==(rhs); }

 private:
  bool check_index() {
    while (chunk_) {
      if (chunk_index_ >= chunk_->size()) {
        chunk_ = next_chunk();
        chunk_index_ = 0;
        if (!chunk_) return false;
      }
      if (chunk_->is_use(chunk_index_)) {
        tuple_ = chunk_->as_tuple<Ts...>(chunk_index_);
        return true;
      }
      ++chunk_index_;
    }
    return false;
  }
  Chunk* next_chunk() {
    auto chunk = chunk_;
    while (chunk) {
      chunk = chunk->next_chunk();
      if (!chunk) return nullptr;
      if (chunk->contains<Ts...>()) return chunk;
    }
    return nullptr;
  }
};

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
    each(f, args_type{});
  }
  template <typename F, typename... Ts>
  void each(F f, type_list<Ts...> args) {
    for (auto chunk = chunk_; chunk; chunk = chunk->next_chunk()) {
      if (chunk->contains<Ts...>()) {
        chunk->each(f, args);
      }
    }
  }

  QueryIterator<Ts...> begin() const { return QueryIterator<Ts...>(chunk_); }
  QueryIterator<Ts...> end() const { return QueryIterator<Ts...>(); }
};

}  // namespace ecs
