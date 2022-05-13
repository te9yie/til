#include <cassert>
#include <iostream>
#include <tuple>

struct Vec3f {
  union {
    float data[3];
    struct {
      float x, y, z;
    };
  };

  template <std::size_t I>
  float get() const {
    return data[I];
  }
  template <std::size_t I>
  float &get() {
    return data[I];
  }
};

template <std::size_t I>
inline float get(const Vec3f &v) {
  return v.data[I];
}
template <std::size_t I>
inline float &get(Vec3f &v) {
  return v.data[I];
}

namespace std {

template <>
struct tuple_size<Vec3f> : integral_constant<size_t, 3> {};

template <size_t I>
struct tuple_element<I, Vec3f> {
  using type = float;
};

}  // namespace std

int main() {
  Vec3f v;
  {
    auto &[x, y, z] = v;
    x = 1;
    y = 2;
    z = 3;
  }
  {
    auto [x, y, z] = v;
    assert(x == 1);
    assert(y == 2);
    assert(z == 3);
    std::cout << x << std::endl;
    std::cout << y << std::endl;
    std::cout << z << std::endl;
  }
}