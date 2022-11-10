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
  task::App app;
  app.context.add_with<int>(1);

  for (int i = 0; i < 10; ++i) {
    app.add_task("func", func);
  }

  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}