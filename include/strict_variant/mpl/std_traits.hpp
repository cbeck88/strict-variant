//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Some C++14 traits that are backported
 */

#include <type_traits>

namespace strict_variant {
namespace mpl {

template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_const_t = typename std::remove_const<T>::type;

template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename T>
using decay_t = typename std::decay<T>::type;

template <bool b, typename U = void>
using enable_if_t = typename std::enable_if<b, U>::type;

// Modified version of std::common_type

// Support: Need a std::decay which allows l-value references to pass through
template <class T>
using mini_decay_t =
  typename std::conditional<std::is_lvalue_reference<T>::value, T, decay_t<T>>::type;

// Our version of common_type uses mini_decay instead of decay
template <typename T, typename... Ts>
struct common_return_type;

template <typename T>
struct common_return_type<T> {
  using type = mini_decay_t<T>;
};

template <typename T1, typename T2>
struct common_return_type<T1, T2> {
  using type = mini_decay_t<decltype(true ? std::declval<T1>() : std::declval<T2>())>;
};

template <typename T1, typename T2, typename T3, typename... Ts>
struct common_return_type<T1, T2, T3, Ts...> {
  using type =
    typename common_return_type<typename common_return_type<T1, T2>::type, T3, Ts...>::type;
};

template <typename T, typename... Ts>
using common_return_type_t = typename common_return_type<T, Ts...>::type;

// This is not actually standard, oh well

template <typename T>
struct is_nothrow_copy_constructible : std::is_nothrow_constructible<T, const T &> {};

} // end namespace mpl
} // end namsepace strict_variant
