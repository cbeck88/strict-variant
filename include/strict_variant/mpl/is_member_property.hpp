//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

namespace strict_variant {
namespace mpl {

/***
 * Property which checks for sameness
 */
template <typename U>
struct sameness {
  template <typename T>
  struct prop : std::is_same<T, U> {};
};

/***
 * Property `not_is_member` checks if an element is in a list
 */
template <typename... Ts>
struct is_member {
  template <typename U>
  struct prop {
    static constexpr bool value = Find_Any<sameness<U>::template prop, Ts...>::value;
  };
};

/***
 * Property `not_is_member` checks if an element is not in a list
 */

template <typename... Ts>
struct not_is_member {
  template <typename U>
  struct prop {
    static constexpr bool value = !is_member<Ts...>::template prop<U>::value;
  };
};

} // end namespace mpl
} // end namespace strict_variant
