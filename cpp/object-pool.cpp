#include <cstdint>
#include <deque>
#include <optional>

struct Id {
  uint32_t index;
  uint32_t revision;

  Id() : index(0), revision(0) {}
  Id(uint32_t i, uint32_t r) : index(i), revision(r) {}

  bool is_invalid() const { return revision == 0; }
  explicit operator bool() const { return revision != 0; }

  bool operator==(const Id& rhs) const {
    return index == rhs.index && revision == rhs.revision;
  }
  bool operator!=(const Id& rhs) const {
    return index != rhs.index || revision != rhs.revision;
  }
};

template <class T, template <class> class Alloc>
class ObjectPool {
 private:
  ObjectPool(const ObjectPool&) = delete;
  ObjectPool& operator=(const ObjectPool&) = delete;
  ObjectPool(ObjectPool&&) = delete;
  ObjectPool& operator=(ObjectPool&&) = delete;

 private:
  struct Storage {
    uint32_t revision;
    std::optional<T> inner;
  };

  std::deque<Storage, Alloc<Storage>> objects_;
  std::deque<uint32_t, Alloc<uint32_t>> revisions_;

 public:
  ObjectPool() = default;

  template <class... Args>
  Id create(Args&&... args) {
    if (revisions_.empty()) {
      auto index = static_cast<uint32_t>(objects_.size());
      auto& obj = objects_.emplace_back();
      obj.revision = 1;
      obj.inner.emplace(std::forward<Args>(args)...);
      return Id{index, obj.revision};
    } else {
      auto index = revisions_.front();
      revisions_.pop_front();
      auto& obj = objects_[index];
      obj.inner.emplace(std::forward<Args>(args)...);
      return Id{index, obj.revision};
    }
  }
  bool destroy(Id id) {
    if (!exists(id)) return false;
    auto& obj = objects_[id.index];
    obj.revision = std::max(obj.revision + 1u, 1u);
    obj.inner.reset();
    revisions_.push_back(id.index);
    return true;
  }

  const T* get(Id id) const {
    if (objects_.size() <= id.index) return nullptr;
    auto& obj = objects_[id.index];
    if (obj.revision != id.revision) return nullptr;
    return &obj.inner.value();
  }
  T* get_mut(Id id) {
    if (objects_.size() <= id.index) return nullptr;
    auto& obj = objects_[id.index];
    if (obj.revision != id.revision) return nullptr;
    return &obj.inner.value();
  }

  bool exists(Id id) const {
    if (objects_.size() <= id.index) return false;
    return objects_[id.index].revision == id.revision;
  }
};

namespace {

struct Foo {
  static int count;
  int id = 0;
  Foo(int id) : id(id) { ++count; }
  Foo(const Foo& foo) : id(foo.id) { ++count; }
  ~Foo() { --count; }
};
int Foo::count = 0;

}  // namespace

#include <cassert>

int main() {
  ObjectPool<Foo, std::allocator> foos;
  assert(Foo::count == 0);

  auto id1 = foos.create(2);
  assert(Foo::count == 1);
  assert(foos.exists(id1));
  assert(foos.get(id1)->id == 2);

  foos.destroy(id1);
  assert(Foo::count == 0);
  assert(!foos.exists(id1));

  auto id2 = foos.create(3);
  assert(Foo::count == 1);
  assert(id1 != id2);
  assert(foos.exists(id2));
  assert(foos.get(id2)->id == 3);

  foos.destroy(id2);
  assert(Foo::count == 0);
  assert(!foos.exists(id2));

  return 0;
}
