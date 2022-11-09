#include "task.h"

namespace task {

/*explicit*/ Task::Task(const TaskPermission& permission)
    : permission_(permission) {}

void Task::exec(const Context& ctx) { on_exec(ctx, &work_); }

bool Task::add_dependency(Task* task) {
  if (is_depended_(task)) return false;
  dependencies_.emplace_back(task);
  return true;
}

bool Task::is_depended_(Task* task) const {
  for (auto it : dependencies_) {
    if (it == task) return true;
    if (it->is_depended_(task)) return true;
  }
  return false;
}

}  // namespace task