#include <cstdlib>
#include <iostream>

#include "app.h"
#include "context.h"
#include "phase.h"
#include "task.h"

int main() {
  task::Context ctx;
  task::Phase phase;

  ctx.add_with<int>(1);

  phase.add_task(std::make_shared<task::FuncTask<int>>(
      [](int i) { std::cout << "int = " << i << std::endl; }));
  phase.add_task(std::make_shared<task::FuncTask<int>>(
      [](int i) { std::cout << "int +2 = " << i + 2 << std::endl; }));

  phase.run(ctx);
  phase.run(ctx);

  task::App app;
  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}