//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * std::remove_reference_t and std::remove_cv_t are part of C++14
 * but they are so usfeul I'm making mpl versions of them.
 */

#include <type_traits>

namespace safe_variant {
namespace mpl {

template <typename T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <typename T>
using remove_const_t = typename std::remove_const<T>::type;

template <typename T>
using remove_cv_t = typename std::remove_cv<T>::type;

template <typename T>
using decay_t = typename std::decay<T>::type;

template <bool b, typename U = void>
using enable_if_t = typename std::enable_if<b, U>::type;

} // end namespace mpl
} // end namsepace safe_variant
