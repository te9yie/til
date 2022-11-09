#pragma once
#include <type_traits>

namespace t9 {

// type_list.
template <typename... Ts>
struct type_list {};

template <typename T, typename... Ts>
struct contains_type;

template <typename T, typename... Ts>
struct contains_type<T, type_list<Ts...>>
    : std::disjunction<std::is_same<T, Ts>...> {};

}  // namespace t9