#pragma once
#include <deque>
#include <memory>

#include "context.h"
#include "t9/type_traits.h"
#include "task.h"

namespace task {

// PhaseId.
using PhaseId = t9::type_int;

// Phase.
template <typename T>
struct Phase {
  static inline const PhaseId id = t9::type2int<T>::value();
};

// PhaseData.
class PhaseData {
 private:
  PhaseId id_ = 0;
  std::string name_;
  std::deque<std::shared_ptr<Task>> tasks_;

 public:
  PhaseData(PhaseId id, std::string_view name) : id_(id), name_(name) {}

  void setup_task_dependencies();
  void run(const Context& ctx);

  void add_task(std::shared_ptr<Task> task);

  PhaseId id() const { return id_; }
  std::string_view name() const { return name_; }
  const std::deque<std::shared_ptr<Task>>& tasks() const { return tasks_; }
};

// make_phase.
template <typename T>
inline std::unique_ptr<PhaseData> make_phase(std::string_view name) {
  return std::make_unique<PhaseData>(Phase<T>::id, name);
}

}  // namespace task