#pragma once
#include <atomic>
#include <cassert>
#include <type_traits>
#include <utility>

namespace t9 {

class RcObject {
 private:
  RcObject(const RcObject&) = delete;
  RcObject(RcObject&&) = delete;
  RcObject& operator=(const RcObject&) = delete;
  RcObject& operator=(RcObject&&) = delete;

 private:
  std::atomic<int> reference_count_ = 1;

 public:
  RcObject() = default;
  virtual ~RcObject() { assert(reference_count_ == 0); }

  void grab() { ++reference_count_; }
  bool drop() {
    if (--reference_count_ > 0) return false;
    on_drop();
    return true;
  }

  int reference_count() const { return reference_count_; }

 protected:
  virtual void on_drop() {}
};

template <class T>
class Rc final {
 private:
  static_assert(std::is_base_of_v<RcObject, T>, "T is not base of RcObject");

 public:
  T* object_ = nullptr;

 public:
  Rc() = default;
  Rc(std::nullptr_t) {}
  Rc(T* p, bool with_grab) : object_(p) {
    if (p && with_grab) {
      p->grab();
    }
  }
  ~Rc() {
    if (object_) {
      object_->drop();
    }
  }

  Rc(const Rc& rhs) : Rc(rhs.object_, true) {}
  Rc& operator=(const Rc& rhs) {
    Rc(rhs).swap(*this);
    return *this;
  }

  Rc(Rc&& rhs) {
    using std::swap;
    swap(object_, rhs.object_);
  }
  Rc& operator=(Rc&& rhs) {
    Rc(std::move(rhs)).swap(*this);
    return *this;
  }

  void swap(Rc& rhs) {
    using std::swap;
    swap(object_, rhs.object_);
  }

  T* get() const { return object_; }
  T* operator->() const { return object_; }
  T& operator*() const { return *object_; }
  explicit operator bool() const { return !!object_; }

  void reset() { Rc().swap(*this); }
  void reset(T* p, bool with_grab) { Rc(p, with_grab).swap(*this); }
  T* release() {
    T* p = object_;
    object_ = nullptr;
    return p;
  }

  bool operator==(const Rc& rhs) const { return object_ == rhs.object_; }
  bool operator!=(const Rc& rhs) const { return object_ != rhs.object_; }
};

template <class T>
inline void swap(Rc<T>& lhs, Rc<T>& rhs) {
  lhs.swap(rhs);
}

}  // namespace t9
