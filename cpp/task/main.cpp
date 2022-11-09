#include <cstdlib>
#include <iostream>

#include "app.h"
#include "context.h"

int main() {
  task::Context<struct App> ctx;

  ctx.add_with<int>(1);
  task::add_with<float>(ctx, 1.5f);

  std::cout << "int = " << *ctx.get<int>() << std::endl;
  std::cout << "float = " << *task::get<float>(ctx) << std::endl;

  task::App app;
  return app.run() ? EXIT_SUCCESS : EXIT_FAILURE;
}