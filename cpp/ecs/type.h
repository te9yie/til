#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <memory>
#include <new>
#include <type_traits>

namespace ecs {

struct Type {
  using Id = std::uintptr_t;
  using CtorFunc = void (*)(void*);
  using DtorFunc = void (*)(void*);

  Id id = 0;
  std::size_t size = 0;
  std::size_t align = 0;
  CtorFunc ctor = nullptr;
  DtorFunc dtor = nullptr;

  template <typename T>
  static Id type2id() {
    static int i = 0;
    return reinterpret_cast<Id>(&i);
  }

  template <typename T>
  static const Type* get() {
    static const Type type = {
        type2id<T>(),
        sizeof(T),
        alignof(T),
        [](void* p) { new (p) T; },
        [](void* p) { std::destroy_at(static_cast<T*>(p)); },
    };
    return &type;
  }
};

template <typename... Ts, std::size_t N>
inline void sort_type_array(const Type* (&types)[N]) {
  std::sort(std::begin(types), std::end(types),
            [](const Type* lhs, const Type* rhs) {
              if (lhs->size < rhs->size) return false;
              if (lhs->size > rhs->size) return true;
              return lhs->id < rhs->id;
            });
}

template <typename T>
using sanitalize_t = std::remove_const_t<std::remove_reference_t<T>>;

}  // namespace ecs
