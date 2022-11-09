#pragma once
#include <cassert>
#include <memory>
#include <unordered_map>

#include "t9/type_traits.h"

namespace task {

// EventReaderIndex.
struct EventReaderIndex {
  std::size_t index = 0;
};

// TaskWork.
class TaskWork {
 private:
  std::unordered_map<t9::type_int, std::unique_ptr<EventReaderIndex>>
      event_reader_indices_;

 public:
  template <typename T>
  std::size_t* event_reader_index_ptr() {
    auto i = t9::type2int<T>::value();
    auto it = event_reader_indices_.find(i);
    if (it == event_reader_indices_.end()) {
      auto p = std::make_unique<EventReaderIndex>();
      auto r = event_reader_indices_.emplace(i, std::move(p));
      assert(r.second);
      it = r.first;
    }
    return &it->second->index;
  }
};

}  // namespace task