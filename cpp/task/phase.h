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
  std::deque<std::shared_ptr<Task>> tasks_;

 public:
  explicit Phase(PhaseId id) : id_(id) {}

  void setup_task_dependencies();
  void run(const Context& ctx);

  void add_task(std::shared_ptr<Task> task);

  PhaseId id() const { return id_; }
};

// make_phase.
template <typename T>
inline std::unique_ptr<Phase> make_phase() {
  return std::make_unique<Phase>(get_phase_id<T>());
}

}  // namespace task