#pragma once
#include <type_traits>

namespace ecs {

template <typename... Ts>
struct type_list {};

namespace detail {

template <typename R, typename... A>
struct function_type_def {
  using result_type = R;
  using args_type = type_list<A...>;
};

template <typename T>
struct function_traits_impl;

template <typename R, typename... A>
struct function_traits_impl<R (*)(A...)> : function_type_def<R, A...> {};

template <typename R, class T, typename... A>
struct function_traits_impl<R (T::*)(A...)> : function_type_def<R, A...> {};

template <typename R, class T, typename... A>
struct function_traits_impl<R (T::*)(A...) const> : function_type_def<R, A...> {
};

template <typename F, typename Dummy = void>
struct function_traits_ : function_traits_impl<F> {};

template <typename F>
struct function_traits_<F, decltype(void(&F::operator()))>
    : function_traits_impl<decltype(&F::operator())> {};

}  // namespace detail

template <typename F>
struct function_traits : detail::function_traits_<std::decay_t<F>> {};

}  // namespace ecs