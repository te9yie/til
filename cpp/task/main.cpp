#include <cstdlib>
#include <iostream>
#include <thread>

#include "app.h"
#include "context.h"
#include "job.h"
#include "phase.h"
#include "task.h"

void func(int i) {
  std::cout << std::this_thread::get_id() << " int = " << i++ << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

int main() {
  task::Context ctx;
  auto executor = ctx.add_with<task::JobExecutor>();
  executor->start(2);

  task::Phase phase;

  ctx.add_with<int>(1);

  for (int i = 0; i < 10; ++i) {
    phase.add_task(std::make_shared<task::FuncTask<int>>(func));
  }

  phase.setup_task_dependencies();
  phase.run(ctx);

  task::App app;
  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}