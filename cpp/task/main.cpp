#include <cstdlib>
#include <iostream>
#include <thread>
#include <unordered_map>

#include "app.h"
#include "context.h"
#include "job.h"
#include "phase.h"
#include "task.h"

void func(int i) {
  std::cout << std::this_thread::get_id() << " int = " << i++ << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
}
void send(task::EventSender<int> sender) { sender.send(2); }
void recv(task::EventReceiver<int> recv) {
  recv.each([](int i) { std::cout << "recv: " << i << std::endl; });
}

void show_tasks(const task::PhaseData* phase) {
  std::unordered_map<const task::Task*, int> task_depth;
  int max_depth = 0;
  for (auto& task : phase->tasks()) {
    int depth = 0;
    if (!task->dependencies().empty()) {
      for (auto& d_task : task->dependencies()) {
        if (auto it = task_depth.find(d_task); it != task_depth.end()) {
          depth = std::max(it->second + 1, depth);
        }
      }
    }
    max_depth = std::max(depth, max_depth);
    task_depth.emplace(task.get(), depth);
  }

  std::unordered_multimap<int, const task::Task*> task_order;
  task_order.clear();
  for (auto it : task_depth) {
    task_order.emplace(it.second, it.first);
  }

  for (int i = 0; i <= max_depth; ++i) {
    std::cout << i << ":\n";
    auto range = task_order.equal_range(i);
    for (auto it = range.first; it != range.second; ++it) {
      std::cout << " [" << it->second->name() << "]" << std::endl;
    }
  }
}
void show_phases(const task::Scheduler* scheduler) {
  auto& phases = scheduler->phases();
  std::for_each(phases.begin(), phases.end(), [](auto& p) {
    std::cout << "Phase: [" << p->name() << "]" << std::endl;
    show_tasks(p.get());
  });
}

int main() {
  task::App app;
  app.context.add_with<int>(1);
  app.add_event<int>();

  app.add_task("send", send);
  for (int i = 0; i < 10; ++i) {
    app.add_task("func", func);
  }
  app.add_task("recv", recv);
  app.add_task_in_phase<task::LastPhase>("show phases", show_phases);

  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}