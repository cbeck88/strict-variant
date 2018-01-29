//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/mpl/ulist.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

namespace strict_variant {
namespace mpl {

// We need a tuple_cdr
// Impl courtesy of Johnathan Wakely
// http://stackoverflow.com/questions/14852593/removing-the-first-type-of-a-stdtuple
// Changed a few things though :)

template <typename T, typename Seq>
struct tuple_cdr_impl;

template <typename T, unsigned I0, unsigned... I>
struct tuple_cdr_impl<T, ulist<I0, I...>> {
  using type = std::tuple<typename std::tuple_element<I, T>::type...>;
};

template <typename T>
struct tuple_cdr : tuple_cdr_impl<T, count_t<std::tuple_size<T>::value>> {};

// Actual impl
template <typename T, unsigned I0, unsigned... I>
typename tuple_cdr<typename std::remove_reference<T>::type>::type
cdr_impl(T && t, ulist<I0, I...>) {
  return typename tuple_cdr<typename std::remove_reference<T>::type>::type{std::get<I>(t)...};
}

template <typename T>
typename tuple_cdr<typename std::remove_reference<T>::type>::type
cdr_tuple(T && t) {
  return cdr_impl(std::forward<T>(t),
                  count_t<std::tuple_size<typename std::remove_reference<T>::type>::value>{});
}

} // end namespace mpl

} // end namespace strict_variant
