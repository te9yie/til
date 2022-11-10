#pragma once
#include <cassert>
#include <functional>
#include <memory>
#include <string>

#include "context.h"
#include "event.h"
#include "scheduler.h"
#include "t9/func_traits.h"
#include "task.h"

namespace task {

//
struct FirstPhase;
struct PreUpdatePhase;
struct UpdatePhase;
struct PostUpdatePhase;
struct LastPhase;

// JobSettings.
struct JobSettings {
  std::size_t thread_n = 0;
};

// App.
class App {
 public:
  using RunnerFunc = std::function<void(App*)>;

 private:
  RunnerFunc runner_;

 public:
  Context context;

 public:
  App();

  void update();
  bool run();

  template <typename F>
  void set_runner(F f) {
    runner_ = f;
  }

  template <typename T>
  void add_event() {
    context.add_with<Event<T>>();
    add_task_in_phase<FirstPhase>("update event",
                                  [](Event<T>* e) { e->update(); });
  }

  template <typename T>
  bool add_phase(std::string_view name) {
    auto phase = make_phase<T>(name);
    return add_phase_(std::move(phase));
  }
  template <typename T>
  bool insert_phase(PhaseId next_phase_id, std::string_view name) {
    auto phase = make_phase<T>(name);
    return insert_phase_(next_phase_id, std::move(phase));
  }

  template <typename PhaseT, typename F>
  bool add_task_in_phase(std::string_view name, F f) {
    using args_type = typename t9::func_traits<F>::args_type;
    return add_task_in_phase_(Phase<PhaseT>::id, name, f, args_type{});
  }
  template <typename F>
  bool add_task(std::string_view name, F f) {
    return add_task_in_phase<UpdatePhase>(name, f);
  }

 private:
  bool setup_();

  bool add_phase_(std::unique_ptr<PhaseData> p);
  bool insert_phase_(PhaseId next_phase_id, std::unique_ptr<PhaseData> p);

  template <typename F, typename... Ts>
  bool add_task_in_phase_(PhaseId phase_id, std::string_view name, F f,
                          t9::type_list<Ts...>) {
    auto task = std::make_shared<FuncTask<Ts...>>(name, f);
    return add_task_in_phase_(phase_id, std::move(task));
  }
  bool add_task_in_phase_(PhaseId phase_id, std::shared_ptr<Task> task);
};

}  // namespace task