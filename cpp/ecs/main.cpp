#include <iostream>

#include "registry.h"

inline std::ostream& operator<<(std::ostream& out, const ecs::EntityId& id) {
  return out << "<" << id.generation << ":" << id.index << ">";
}

int main() {
  ecs::Registry reg;

  reg.create_entity<int, float>();
  reg.create_entity<int, float>();
  reg.create_entity<float, double>();
  reg.create_entity<int, double>();

  auto query = reg.query<ecs::EntityId, int&>();

  query.each([](ecs::EntityId id, int& i) {
    std::cout << "ID:" << id << " = " << i << std::endl;
    i = 2;
  });
  for (auto [id, i] : query) {
    std::cout << id << "i: " << i << std::endl;
    i = 4;
  }
  for (auto [id, i] : query) {
    std::cout << id << " = i: " << i << std::endl;
  }
}
