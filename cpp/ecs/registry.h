#pragma once
#include <deque>
#include <memory>
#include <vector>

#include "archetype.h"
#include "chunk.h"
#include "entity.h"
#include "function_traits.h"
#include "query.h"

namespace ecs {

class Registry final {
 private:
  Registry(const Registry&) = delete;
  Registry(Registry&&) = delete;
  Registry& operator=(const Registry&) = delete;
  Registry& operator=(Registry&&) = delete;

 private:
  std::vector<EntityStorage> entities_;
  std::deque<std::size_t> free_indices_;
  std::vector<std::unique_ptr<Archetype>> archetypes_;
  std::vector<std::unique_ptr<Chunk>> chunks_;
  Chunk* for_iter_ = nullptr;

 public:
  Registry() = default;
  ~Registry() { destroy_all_entities(); }

  template <typename... Ts>
  EntityId create_entity() {
    auto archetype = get_or_new_archetype<EntityId, Ts...>();
    auto chunk = get_or_new_chunk(archetype);
    auto index = create_entity_index();
    auto chunk_index = chunk->create();

    EntityId id = {entities_[index].generation, index};
    entities_[index].chunk = chunk;
    entities_[index].chunk_index = chunk_index;
    *chunk->get<EntityId>(chunk_index) = id;

    return id;
  }

  bool destroy_entity(EntityId id) {
    if (id.index >= entities_.size()) return false;
    if (id.generation != entities_[id.index].generation) return false;

    auto& storage = entities_[id.index];
    storage.chunk->destroy(storage.chunk_index);

    free_indices_.push_back(id.index);
    storage.generation = std::max<size_t>(id.generation + 1, 1);
    storage.chunk = nullptr;
    storage.chunk_index = 0;

    return true;
  }

  void destroy_all_entities() {
    for (auto& entity : entities_) {
      if (!entity.chunk) continue;
      entity.chunk->destroy(entity.chunk_index);
    }
    entities_.clear();
    free_indices_.clear();
  }

  template <typename... Ts>
  Query<Ts...> query() {
    return Query<Ts...>(for_iter_);
  }

 private:
  template <typename... Ts>
  Archetype* get_or_new_archetype() {
    for (auto& archetype : archetypes_) {
      if (archetype->is_match<Ts...>()) {
        return archetype.get();
      }
    }
    auto archetype = Archetype::make<Ts...>();
    auto p = archetype.get();
    archetypes_.emplace_back(std::move(archetype));
    return p;
  }

  Chunk* get_or_new_chunk(Archetype* archetype) {
    if (auto chunk = archetype->get_free_chunk()) {
      return chunk;
    }
    std::unique_ptr<Chunk> chunk(new Chunk(archetype->tuple()));
    auto p = chunk.get();
    chunks_.emplace_back(std::move(chunk));
    link_chunk(p);
    archetype->link_chunk(p);
    return p;
  }

  std::size_t create_entity_index() {
    size_t index = 0;
    if (free_indices_.empty()) {
      index = entities_.size();
      entities_.emplace_back().generation = 1;
    } else {
      index = free_indices_.front();
      free_indices_.pop_front();
    }
    return index;
  }

  void link_chunk(Chunk* chunk) {
    for (auto it = for_iter_; it; it = it->next_chunk()) {
      if (it->next_chunk()) continue;
      it->link_chunk(chunk);
      return;
    }
    for_iter_ = chunk;
  }
};

}  // namespace ecs
