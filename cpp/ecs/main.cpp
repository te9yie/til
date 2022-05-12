#include <iostream>

#include "registry.h"

inline std::ostream& operator<<(std::ostream& out, const ecs::EntityId& id) {
  return out << "<" << id.generation << ":" << id.index << ">";
}

int main() {
  ecs::Registry reg;

  auto id = reg.create_entity<int, float>();
  auto query = reg.query<int>();
  query.each([](ecs::EntityId id, int& i) {
    std::cout << "ID:" << id << " = " << i << std::endl;
    i = 2;
  });
  query.each([](ecs::EntityId id, const int& i) {
    std::cout << "ID:" << id << " = " << i << std::endl;
  });
  /*
  for (auto& [id, i, f] : query.iter()) {
  }
  */

  reg.destroy_entity(id);
}
