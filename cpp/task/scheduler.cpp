#include "scheduler.h"

#include <algorithm>

namespace task {

void Scheduler::setup_task_dependencies() {
  std::for_each(phases_.begin(), phases_.end(),
                [](auto& p) { p->setup_task_dependencies(); });
}
void Scheduler::run(const Context& ctx) {
  std::for_each(phases_.begin(), phases_.end(),
                [&ctx](auto& p) { p->run(ctx); });
}

bool Scheduler::add_phase(std::unique_ptr<PhaseData> p) {
  phases_.emplace_back(std::move(p));
  return true;
}
bool Scheduler::insert_phase(PhaseId next_id, std::unique_ptr<PhaseData> p) {
  for (auto it = phases_.begin(); it != phases_.end(); ++it) {
    auto& phase = *it;
    if (next_id == phase->id()) {
      phases_.insert(it, std::move(p));
      return true;
    }
  }
  return false;
}

bool Scheduler::add_task(PhaseId id, std::shared_ptr<Task> task) {
  auto it = std::find_if(phases_.begin(), phases_.end(),
                         [id](auto& p) { return id == p->id(); });
  if (it == phases_.end()) return false;
  auto& phase = *it;
  phase->add_task(std::move(task));
  return true;
}

}  // namespace task