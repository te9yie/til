#pragma once
#include "context.h"
#include "t9/type_list.h"
#include "task_permission.h"
#include "task_work.h"

namespace task {

// task_traits.
template <typename T>
struct task_traits {
  static void set_permission(TaskPermission* permission) {
    permission->set_read<T>();
  }
  static T apply_args(const Context& ctx, TaskWork* /*work*/) {
    return *ctx.get<T>();
  }
};
template <typename T>
struct task_traits<T&> {
  static void set_permission(TaskPermission* permission) {
    permission->set_write<T>();
  }
  static T& apply_args(const Context& ctx, TaskWork* /*work*/) {
    return *ctx.get<T>();
  }
};
template <typename T>
struct task_traits<const T&> {
  static void set_permission(TaskPermission* permission) {
    permission->set_read<T>();
  }
  static const T& apply_args(const Context& ctx, TaskWork* /*work*/) {
    return *ctx.get<T>();
  }
};
template <typename T>
struct task_traits<T*> {
  static void set_permission(TaskPermission* permission) {
    permission->set_write<T>();
  }
  static T* apply_args(const Context& ctx, TaskWork* /*work*/) {
    return ctx.get<T>();
  }
};
template <typename T>
struct task_traits<const T*> {
  static void set_permission(TaskPermission* permission) {
    permission->set_read<T>();
  }
  static const T* apply_args(const Context& ctx, TaskWork* /*work*/) {
    return ctx.get<T>();
  }
};

namespace detail {

inline void set_task_permission(TaskPermission*, t9::type_list<>) {}

template <typename T, typename... Ts>
inline void set_task_permission(TaskPermission* permission,
                                t9::type_list<T, Ts...>) {
  task_traits<T>::set_permission(permission);
  set_task_permission(permission, t9::type_list<Ts...>{});
}

}  // namespace detail

// make_task_permission.
template <typename... Ts>
inline TaskPermission make_task_permission() {
  TaskPermission permission;
  detail::set_task_permission(&permission, t9::type_list<Ts...>{});
  return permission;
}

}  // namespace task