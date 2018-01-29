//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/mpl/std_traits.hpp>
#include <type_traits>

namespace strict_variant {

/***
 * Metafunction `safe_pointer_conversion`
 *
 * Type relationship on two parameters, both should be pointer types.
 *
 * Identifies a subset of conversions which are permitted during variant construction.
 */

//[ strict_variant_safe_pointer_conversion
template <typename A, typename B>
struct safe_pointer_conversion;

template <typename A, typename B>
struct safe_pointer_conversion<A *, B *> {
  static constexpr bool value = (std::is_same<A, B>::value || std::is_same<A, const B>::value)
                                && std::is_constructible<A *, B *>::value;
};
//]

} // end namespace strict_variant
