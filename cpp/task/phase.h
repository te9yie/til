#pragma once
#include <deque>
#include <memory>

#include "context.h"
#include "task.h"

namespace task {

// Phase.
class Phase {
 private:
  std::deque<std::shared_ptr<Task>> tasks_;

 public:
  void run(const Context& ctx);
  void add_task(std::shared_ptr<Task> task);
  void setup_task_dependencies();
};

}  // namespace task