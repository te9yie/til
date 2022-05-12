#pragma once
#include <cassert>
#include <cstddef>
#include <memory>

#include "type.h"

namespace ecs {

class Tuple {
 private:
  Tuple(const Tuple&) = delete;
  Tuple(Tuple&&) = delete;
  Tuple& operator=(const Tuple&) = delete;
  Tuple& operator=(Tuple&&) = delete;

 private:
  struct Element {
    const Type* type = nullptr;
    std::size_t offset = 0;
  };

 private:
  std::unique_ptr<Element[]> types_;
  std::size_t type_size_ = 0;
  std::size_t memory_size_ = 0;

 private:
  template <std::size_t N>
  Tuple(const Type* (&types)[N])
      : types_(std::make_unique<Element[]>(N)), type_size_(N) {
    const auto fix_align = [](std::size_t offset, std::size_t align) {
      auto r = offset % align;
      return r == 0 ? offset : offset + align - r;
    };
    size_t offset = 0;
    size_t align = 0;
    for (std::size_t i = 0; i < N; ++i) {
      offset = fix_align(offset, types[i]->align);
      types_[i].type = types[i];
      types_[i].offset = offset;
      offset += types[i]->size;
      align = std::max(align, types[i]->align);
    }
    offset = fix_align(offset, align);
    memory_size_ = offset;
  }

 public:
  void construct(void* p) const {
    auto top = reinterpret_cast<std::uint8_t*>(p);
    for (std::size_t i = 0; i < type_size_; ++i) {
      types_[i].type->ctor(top + types_[i].offset);
    }
  }
  void destruct(void* p) const {
    auto top = reinterpret_cast<std::uint8_t*>(p);
    for (std::size_t i = 0; i < type_size_; ++i) {
      types_[i].type->dtor(top + types_[i].offset);
    }
  }

  template <typename T>
  bool try_get_offset(std::size_t* out_offset) const {
    assert(out_offset);
    auto type = Type::get<T>();
    for (std::size_t i = 0; i < type_size_; ++i) {
      if (types_[i].type == type) {
        *out_offset = types_[i].offset;
        return true;
      }
    }
    return false;
  }

  std::size_t memory_size() const { return memory_size_; }

  template <typename... Ts>
  bool is_match() const {
    std::size_t n = sizeof...(Ts);
    if (type_size_ != n) return false;
    const Type* types[] = {Type::get<sanitalize_t<Ts>>()...};
    sort_type_array(types);
    for (std::size_t i = 0; i < n; ++i) {
      if (types_[i].type != types[i]) return false;
    }
    return false;
  }

  template <typename... Ts>
  bool contains() const {
    std::size_t n = sizeof...(Ts);
    if (type_size_ < n) return false;
    const Type* types[] = {Type::get<sanitalize_t<Ts>>()...};
    sort_type_array(types);
    for (std::size_t i = 0, j = 0; i < type_size_; ++i) {
      if (types_[i].type == types[j]) {
        if (++j == n) return true;
      }
    }
    return false;
  }

  template <class... Ts>
  static std::unique_ptr<Tuple> make() {
    const Type* types[] = {Type::get<sanitalize_t<Ts>>()...};
    sort_type_array(types);
    return std::unique_ptr<Tuple>(new Tuple(types));
  }
};

}  // namespace ecs
