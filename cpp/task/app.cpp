#include "app.h"

#include <cassert>

#include "job.h"

namespace task {

App::App() {
  context.add_with<JobExecutor>();
  auto scheduler = context.add_with<Scheduler>();

  scheduler->add_phase(make_phase<FirstPhase>("FirstPhase"));
  scheduler->add_phase(make_phase<PreUpdatePhase>("PreUpdatePhase"));
  scheduler->add_phase(make_phase<UpdatePhase>("UpdatePhase"));
  scheduler->add_phase(make_phase<PostUpdatePhase>("PostUpdatePhase"));
  scheduler->add_phase(make_phase<LastPhase>("LastPhase"));
}

void App::update() {
  auto scheduler = context.get<Scheduler>();
  scheduler->run(context);
}

bool App::run() {
  if (!setup_()) return false;
  if (runner_) {
    runner_(this);
  } else {
    update();
  }
  return true;
}

bool App::setup_() {
  std::size_t thread_n = std::thread::hardware_concurrency();
  if (auto settings = context.get<JobSettings>()) {
    thread_n = settings->thread_n;
  }
  auto executor = context.get<JobExecutor>();
  assert(executor);
  if (!executor->start(thread_n)) return false;

  if (auto scheduler = context.get<Scheduler>()) {
    scheduler->setup_task_dependencies();
  }

  return true;
}

bool App::add_phase_(std::unique_ptr<PhaseData> p) {
  auto scheduler = context.get<Scheduler>();
  return scheduler->add_phase(std::move(p));
}
bool App::insert_phase_(PhaseId next_phase_id, std::unique_ptr<PhaseData> p) {
  auto scheduler = context.get<Scheduler>();
  return scheduler->insert_phase(next_phase_id, std::move(p));
}

bool App::add_task_in_phase_(PhaseId phase_id, std::shared_ptr<Task> task) {
  auto scheduler = context.get<Scheduler>();
  return scheduler->add_task(phase_id, std::move(task));
}

}  // namespace task