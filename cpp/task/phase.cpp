#include "phase.h"

#include <iterator>

namespace task {

void Phase::run(const Context& ctx) {
  for (auto& task : tasks_) {
    task->exec(ctx);
  }
}

void Phase::add_task(std::shared_ptr<Task> task) {
  tasks_.emplace_back(std::move(task));
}

void Phase::setup_task_dependencies() {
  for (auto it = tasks_.begin(), end = tasks_.end(); it != end; ++it) {
    using rev_iter = std::reverse_iterator<decltype(it)>;
    auto task = it->get();
    const auto& permission = task->permission();
    for (auto i : permission.writes) {
      for (auto r_it = rev_iter(it), r_end = tasks_.rend(); r_it != r_end;
           ++r_it) {
        if ((*r_it)->permission().is_conflict_write(i)) {
          task->add_dependency(r_it->get());
        }
      }
    }
    for (auto i : permission.reads) {
      for (auto r_it = rev_iter(it), r_end = tasks_.rend(); r_it != r_end;
           ++r_it) {
        if ((*r_it)->permission().is_conflict_read(i)) {
          task->add_dependency(r_it->get());
        }
      }
    }
  }
}

}  // namespace task