//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Some C++14 traits that are backported
 */

#include <type_traits>
#include <utility>

namespace safe_variant {
namespace mpl {

template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_const_t = typename std::remove_const<T>::type;

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <typename T>
using remove_pointer_t = typename std::remove_pointer<T>::type;

template <typename T>
using decay_t = typename std::decay<T>::type;

template <bool b, typename U = void>
using enable_if_t = typename std::enable_if<b, U>::type;

// Common Type backport

template <typename T, typename... Ts>
struct common_type;

template <typename T>
struct common_type<T> {
  using type = decay_t<T>;
};

template <typename T1, typename T2>
struct common_type<T1, T2> {
  using type = decay_t<decltype(true ? std::declval<T1>() : std::declval<T2>())>;
};

template <typename T>
struct common_type<T, T> {
  using type = T;
};

template <typename T1, typename T2, typename T3, typename... Ts>
struct common_type<T1, T2, T3, Ts...> {
  using type = typename common_type<typename common_type<T1, T2>::type, T3, Ts...>::type;
};

template <typename T, typename... Ts>
using common_type_t = typename common_type<T, Ts...>::type;

} // end namespace mpl
} // end namsepace safe_variant
