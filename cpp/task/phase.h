#pragma once
#include <deque>
#include <memory>

#include "context.h"
#include "t9/type_traits.h"
#include "task.h"

namespace task {

// PhaseId.
using PhaseId = t9::type_int;

// get_phase_id.
template <typename T>
inline PhaseId get_phase_id() {
  return t9::type2int<T>::value();
}

// Phase.
class Phase {
 private:
  PhaseId id_ = 0;
  std::string name_;
  std::deque<std::shared_ptr<Task>> tasks_;

 public:
  Phase(PhaseId id, std::string_view name) : id_(id), name_(name) {}

  void setup_task_dependencies();
  void run(const Context& ctx);

  void add_task(std::shared_ptr<Task> task);

  PhaseId id() const { return id_; }
  std::string_view name() const { return name_; }
  const std::deque<std::shared_ptr<Task>>& tasks() const { return tasks_; }
};

// make_phase.
template <typename T>
inline std::unique_ptr<Phase> make_phase(std::string_view name) {
  return std::make_unique<Phase>(get_phase_id<T>(), name);
}

}  // namespace task