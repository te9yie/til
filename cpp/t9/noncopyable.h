#pragma once

namespace t9 {

// prevent unintended ADL.
namespace noncopyable_ {

// NonCopyable.
class NonCopyable {
 private:
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;

 protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};

}  // namespace noncopyable_

using NonCopyable = noncopyable_::NonCopyable;

}  // namespace t9