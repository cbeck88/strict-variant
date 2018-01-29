//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/mpl/std_traits.hpp>
#include <type_traits>
#include <utility>

namespace strict_variant {

//[ strict_variant_is_recursive_wrapper
namespace detail {

/***
 * Trait to identify wrapper types like recursive_wrapper.
 * Specialize this trait to make strict_variant recognize and automatically
 * pierce custom wrappers.
 */

template <typename T>
struct is_wrapper : std::false_type {};

} // end namespace detail
//]

//[ strict_variant_pierce_wrapper
namespace detail {

/***
 * Function to pierce a wrapper
 */

template <typename T>
inline auto
pierce_wrapper(T && t)
  -> mpl::enable_if_t<!is_wrapper<mpl::remove_const_t<mpl::remove_reference_t<T>>>::value, T> {
  return std::forward<T>(t);
}

template <typename T>
inline auto
pierce_wrapper(T && t)
  -> mpl::enable_if_t<is_wrapper<mpl::remove_const_t<mpl::remove_reference_t<T>>>::value,
                      decltype(std::forward<T>(t).get())> {
  return std::forward<T>(t).get();
}

} // end namespace detail
  //]

/***
 * Trait to remove a wrapper from a wrapped type
 */
template <typename T, bool is_wrapped = detail::is_wrapper<T>::value>
struct unwrap_type;

template <typename T>
struct unwrap_type<T, false> {
  typedef T type;
};

template <typename T>
struct unwrap_type<T, true> {
  typedef typename T::value_type type;
};

template <typename T>
using unwrap_type_t = typename unwrap_type<T>::type;

} // end namespace strict_variant
