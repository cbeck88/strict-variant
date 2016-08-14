//  (C) Copyright 2016 Christopher Beck

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
  static constexpr bool unsign_to_sign = !mpl::is_unsigned<A>::value && mpl::is_unsigned<B>::value;

  static constexpr int ra = mpl::arithmetic_rank<A>::value;
  static constexpr int rb = mpl::arithmetic_rank<B>::value;

  static constexpr bool value = same_class && !unsign_to_sign && (ra >= rb);
};
//]

/***
 * Metafunction `safely_constructible`:
 *   A metafunction yielding a bool which detects is A is constructible from B without using
 *   conversions that we consider "unsafe". Roughly this means, information could potentially
 *   be lost on some platform.
 *
 *   Typically, this function is the same as `std::is_constructible` except in regards to
 *   conversions of fundamental types and pointer types.
 *
 *   The main purpose of this trait is to support `strict_variant::variant` in deciding what
 *   conversions to allow, and how to deduce what internal type to assign to any given type.
 *
 *   Between numeric types:
 *     We forbid converting between integral and floating point.
 *     We forbid converting unsigned to signed.
 *     We forbid converting between bool and non-bool.
 *     We forbid conversions which are potentially lossy in some conforming
 *       implementation of C++.
 *       For instance, long to int is not allowed because the standard permits
 *       long to be strictly larger than int. int to long is allowed, because it can never be
 *       lossy.
 *     We permit signed to unsigned and smaller to larger, but not simultaneously.
 *     When either of the types being converted is not primitive, this is the
 *       same as std::is_constructible.
 *   Between pointer types:
 *     We forbid conversions, except for those involving only CV qualifiers and references, and
 *     decay of array -> pointer, etc.
 *
 *   We forbid all conversions between numeric types and pointer types.
 *
 *   Safely-constructible is intended to be a pre-order, that is, if B is safely constructible from
 *   A, or there is a sequence of safely constructible types leading from A to B, this shoud not
 *   also be the case for B to A, unless A = B (modulo CV).
 *
 *   It is not a partial order. We allow unsigned int -> unsigned long, and int -> unsigned int,
 *   but not int -> unsigned long. You have to do that in two steps.
 */

//[ strict_variant_safely_constructible
// Helper metafunction which checks if type is arithmetic after decay.
// This is the same as, `is_arithmetic` modulo `const` and references.
template <typename T>
struct decay_to_arithmetic : std::is_arithmetic<mpl::decay_t<T>> {};

// Helper metafunction which checks if type is pointer after decay,
template <typename T>
struct decay_to_ptr : std::is_pointer<mpl::decay_t<T>> {};

// Primary template, falls back to std::is_constructible
template <typename A, typename B, typename ENABLE = void>
struct safely_constructible : std::is_constructible<A, B> {};

// If both are arithmetic, then decay and pass to safe_arithmetic_conversion
template <typename A, typename B>
struct safely_constructible<A, B, mpl::enable_if_t<decay_to_arithmetic<A>::value
                                                   && decay_to_arithmetic<B>::value>>
  : safe_arithmetic_conversion<mpl::decay_t<A>, mpl::decay_t<B>> {};

// If both are pointer after decay, then check if they are the same or represent T * -> const T *.
template <typename A, typename B>
struct safely_constructible<A, B,
                            mpl::enable_if_t<decay_to_ptr<A>::value && decay_to_ptr<B>::value>> {
  using A2 = mpl::decay_t<A>;
  using B2 = mpl::decay_t<B>;
  static constexpr bool value =
    (std::is_same<A2, B2>::value || std::is_same<mpl::remove_const_t<mpl::remove_pointer_t<A2>>,
                                                 mpl::remove_pointer_t<B2>>::value)
    && std::is_constructible<A, B>::value;
};

// If one is arithmetic and the other is pointer, after decay, it is forbidden.
template <typename A, typename B>
struct safely_constructible<A, B, mpl::enable_if_t<(decay_to_arithmetic<A>::value
                                                    && decay_to_ptr<B>::value)
                                                   || (decay_to_ptr<A>::value
                                                       && decay_to_arithmetic<B>::value)>> {
  static constexpr bool value = false;
};
//]

} // end namespace strict_variant
