#pragma once

#include <array>
#include <cassert>
#include <vector>

#include "task_traits.h"
#include "task_work.h"

namespace task {

// Event.
template <typename T>
class Event {
 public:
  // EventData.
  struct EventData {
    size_t index;
    T x;

    template <class... Args>
    EventData(size_t index, Args&&... args)
        : index(index), x(std::forward<Args>(args)...) {}
  };

  static constexpr std::size_t FRAME_N = 2;

 private:
  std::array<std::vector<EventData>, FRAME_N> events_;
  std::array<std::size_t, FRAME_N> start_index_ = {0};
  std::size_t count_ = 0;
  std::size_t frame_index_ = 0;

 public:
  virtual ~Event() = default;

  void update() {
    auto index = (frame_index_ + 1) % FRAME_N;
    events_[index].clear();
    start_index_[index] = count_;
    frame_index_ = index;
  }

 public:
  template <class... Args>
  void send(Args&&... args) {
    events_[frame_index_].emplace_back(count_++, std::forward<Args>(args)...);
  }

  const EventData* get_event(std::size_t index) const {
    auto n = count_ - index;
    if (n == 0) return nullptr;
    auto f_i = frame_index_;
    do {
      if (n <= events_[f_i].size()) {
        auto i = index - start_index_[f_i];
        return &events_[f_i][i];
      }
      n -= events_[f_i].size();
      f_i = (f_i + FRAME_N - 1) % FRAME_N;
    } while (f_i != frame_index_);
    return nullptr;
  }
};

// EventSender.
template <typename T>
struct EventSender {
  Event<T>* events = nullptr;

  template <class... Args>
  void send(Args&&... args) {
    assert(events);
    events->send(std::forward<Args>(args)...);
  }
};

// EventReceiver.
template <typename T>
struct EventReceiver {
  Event<T>* events = nullptr;
  std::size_t* index = nullptr;

  template <typename F>
  void each(F f) {
    assert(events);
    for (auto e = events->get_event(*index); e; e = events->get_event(*index)) {
      *index = e->index + 1;
      f(e->x);
    }
  }
};

// task_traits.
template <typename T>
struct task_traits<EventSender<T>> {
  static void set_permission(TaskPermission* permission) {
    permission->set_write<Event<T>>();
  }
  static EventSender<T> apply_args(const Context* ctx, TaskWork* /*work*/) {
    return EventSender<T>{ctx->get<Event<T>>()};
  }
};
template <typename T>
struct task_traits<EventReceiver<T>> {
  static void set_permission(TaskPermission* permission) {
    permission->set_read<Event<T>>();
  }
  static EventReceiver<T> apply_args(const Context* ctx, TaskWork* work) {
    return EventReceiver<T>{ctx->get<Event<T>>(),
                            work->event_reader_index_ptr<T>()};
  }
};

}  // namespace task