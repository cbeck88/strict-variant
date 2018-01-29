//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/conversion_rank.hpp>
#include <strict_variant/mpl/std_traits.hpp>
#include <type_traits>

namespace strict_variant {

/***
 * Metafunction `safe_arithmetic_conversion`
 *
 * Type relationship on two parameters, both should be arithmetic types.
 *
 * Identifies a subset of conversions which
 * - Do not cross the "arithmetic categories" that we defined.
 * - Are *totally portable*: No narrowing or implementation-defined behavior.
 *
 * This means:
 *   - Arithmetic category must be the same
 *   - Arithmetic rank must be nondecreasing
 *   - Unsigned -> Signed is not allowed.
 */

//[ strict_variant_safe_arithmetic_conversion
template <typename A, typename B>
struct safe_arithmetic_conversion {
  static constexpr bool same_class =
    (mpl::classify_arithmetic<A>::value == mpl::classify_arithmetic<B>::value);
  static constexpr bool unsign_to_sign = !std::is_unsigned<A>::value && std::is_unsigned<B>::value;
  static constexpr bool narrowing =
    (mpl::arithmetic_rank<A>::value < mpl::arithmetic_rank<B>::value);

  static constexpr bool value = same_class && !unsign_to_sign && !narrowing;
};

template <typename A>
struct safe_arithmetic_conversion<A, char> {
  static constexpr bool value = safe_arithmetic_conversion<A, signed char>::value
                                && safe_arithmetic_conversion<A, unsigned char>::value;
};

template <typename B>
struct safe_arithmetic_conversion<char, B> {
  static constexpr bool value = safe_arithmetic_conversion<signed char, B>::value
                                && safe_arithmetic_conversion<unsigned char, B>::value;
};

template <>
struct safe_arithmetic_conversion<char, char> : std::true_type {};
//]

} // end namespace strict_variant
