#pragma once
#include <memory>
#include <vector>

#include "t9/singleton.h"

namespace task {

// Context.
class Context : public t9::Singleton<Context> {
 private:
  // TypeIndex.
  template <typename T>
  struct TypeIndex {
    static inline std::size_t index = 0;
  };

  // EntryBase.
  class EntryBase {
   public:
    virtual ~EntryBase() = default;
  };

  // Entry.
  template <typename T>
  class Entry : public EntryBase {
   public:
    std::unique_ptr<T> x;
  };

 private:
  std::vector<std::unique_ptr<EntryBase>> entries_;

 public:
  virtual ~Context() {
    while (!entries_.empty()) {
      entries_.pop_back();
    }
  }

  template <typename T>
  T* add(std::unique_ptr<T> x) {
    auto i = TypeIndex<T>::index;
    if (i != 0) return nullptr;
    auto p = x.get();
    auto e = std::make_unique<Entry<T>>();
    e->x = std::move(x);
    entries_.emplace_back(std::move(e));
    TypeIndex<T>::index = entries_.size();
    return p;
  }

  template <typename T, typename... Args>
  T* add_with(Args&&... args) {
    return add(std::make_unique<T>(std::forward<Args>(args)...));
  }

  template <typename T>
  T* get() const {
    auto i = TypeIndex<T>::index;
    if (i == 0) return nullptr;
    auto e = static_cast<Entry<T>*>(entries_[i - 1].get());
    return e->x.get();
  }

  template <typename T>
  std::size_t index() const {
    return TypeIndex<T>::index;
  }
};

}  // namespace task