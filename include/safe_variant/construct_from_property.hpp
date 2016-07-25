//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * construct_from property -- checks if a fixed type can be used to construct
 * given type.
 */

#include <type_traits>

namespace safe_variant {
namespace mpl {

template <typename T>
struct construct_from {
  template <typename U>
  struct prop {
    static constexpr bool value = std::is_constructible<U, T>::value;
  };
};

} // end namespace mpl
} // end namespace safe_variant
