#pragma once
#include <deque>
#include <functional>
#include <string>

#include "context.h"
#include "job.h"
#include "t9/noncopyable.h"
#include "task_permission.h"
#include "task_traits.h"
#include "task_work.h"

namespace task {

// Task.
class Task : private t9::NonCopyable, public Job {
 private:
  TaskPermission permission_;
  TaskWork work_;
  std::deque<Task*> dependencies_;
  const Context* context_ = nullptr;

 public:
  Task(std::string_view name, const TaskPermission& permission);
  virtual ~Task() = default;

  void set_context(const Context* ctx) { context_ = ctx; }

  const TaskPermission& permission() const { return permission_; }

  bool add_dependency(Task* task);
  std::deque<Task*> dependencies() const { return dependencies_; }

 protected:
  // Job.
  virtual bool on_can_exec() const override;
  virtual void on_exec() override;

 protected:
  virtual void on_exec_task(const Context* ctx, TaskWork* work) = 0;

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
  FuncTask(std::string_view name, FuncType f)
      : Task(name, make_task_permission<Args...>()), func_(f) {}

 protected:
  virtual void on_exec_task(const Context* ctx, TaskWork* work) override {
    func_(task_traits<Args>::apply_args(ctx, work)...);
  }
};

}  // namespace task