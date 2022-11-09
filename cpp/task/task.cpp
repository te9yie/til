#include "task.h"

namespace task {

/*explicit*/ Task::Task(const TaskPermission& permission)
    : permission_(permission) {}

bool Task::add_dependency(Task* task) {
  if (is_depended_(task)) return false;
  dependencies_.emplace_back(task);
  return true;
}

/*virtual*/ bool Task::on_can_exec() const /*override*/ {
  for (auto task : dependencies_) {
    if (!task->is_state(State::Done)) return false;
  }
  return true;
}
/*virtual*/ void Task::on_exec() /*override*/ {
  on_exec_task(context_, &work_);
}

bool Task::is_depended_(Task* task) const {
  for (auto it : dependencies_) {
    if (it == task) return true;
    if (it->is_depended_(task)) return true;
  }
  return false;
}

}  // namespace task