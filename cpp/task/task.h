#pragma once
#include <deque>
#include <functional>

#include "context.h"
#include "t9/noncopyable.h"
#include "task_permission.h"
#include "task_traits.h"
#include "task_work.h"

namespace task {

// Task.
class Task : private t9::NonCopyable {
 private:
  TaskPermission permission_;
  TaskWork work_;
  std::deque<Task*> dependencies_;

 public:
  explicit Task(const TaskPermission& permission);
  virtual ~Task() = default;

  void exec(const Context& ctx);

  const TaskPermission& permission() const { return permission_; }

  bool add_dependency(Task* task);
  std::deque<Task*> dependencies() const { return dependencies_; }

 protected:
  virtual void on_exec(const Context& ctx, TaskWork* work) = 0;

 private:
  bool is_depended_(Task* task) const;
};

// FuncTask.
template <typename... Args>
class FuncTask : public Task {
 private:
  using FuncType = std::function<void(Args...)>;

 private:
  FuncType func_;

 public:
  explicit FuncTask(FuncType f)
      : Task(make_task_permission<Args...>()), func_(f) {}

 protected:
  virtual void on_exec(const Context& ctx, TaskWork* work) override {
    func_(task_traits<Args>::apply_args(ctx, work)...);
  }
};

}  // namespace task