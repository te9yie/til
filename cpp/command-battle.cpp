#include <iostream>
#include <string>

struct Status {
  std::string name;
  int hit_point;
  int strength;
};

const Status status_data[] = {
    // clang-format off
    { "cat", 100, 100, },
    { "dog", 100, 100, },
    // clang-format on
};

int main() {
  for (auto status : status_data) {
    std::cout << "---\n"
              << "Name: " << status.name << "\n"
              << "HP: " << status.hit_point << "\n"
              << "STR: " << status.strength << std::endl;
  }
  return 0;
}