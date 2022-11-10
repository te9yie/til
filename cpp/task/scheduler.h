#pragma once
#include <list>
#include <memory>

#include "phase.h"

namespace task {

// Scheduler.
class Scheduler {
 private:
  std::list<std::unique_ptr<PhaseData>> phases_;

 public:
  void setup_task_dependencies();
  void run(const Context& ctx);

  bool add_phase(std::unique_ptr<PhaseData> p);
  bool insert_phase(PhaseId next_id, std::unique_ptr<PhaseData> p);

  bool add_task(PhaseId id, std::shared_ptr<Task> task);

  const std::list<std::unique_ptr<PhaseData>>& phases() const {
    return phases_;
  }
};

}  // namespace task