//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/conversion_rank.hpp>
#include <strict_variant/mpl/std_traits.hpp>
#include <type_traits>

namespace strict_variant {

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

// Helper metafunction which checks if type is arithmetic after decay.
template <typename T>
struct is_numeric : std::is_arithmetic<mpl::decay_t<T>> {};

// Helper metafunction which checks if type is pointer after decay,
template <typename T>
struct is_ptr : std::is_pointer<mpl::decay_t<T>> {};

// Primary template, falls back to std::is_constructible
template <typename A, typename B, typename ENABLE = void>
struct safely_constructible : std::is_constructible<A, B> {};

// If both are numeric, then decay and pass to safe_by_rank
template <typename A, typename B>
struct safely_constructible<A, B, mpl::enable_if_t<is_numeric<A>::value && is_numeric<B>::value>>
  : public mpl::safe_by_rank<mpl::decay_t<A>,
                             mpl::decay_t<B>> {};

// If both are pointer after decay, then check if they are the same or represent T * -> const T *.
template <typename A, typename B>
struct safely_constructible<A, B, mpl::enable_if_t<is_ptr<A>::value && is_ptr<B>::value>> {
  using A2 = mpl::decay_t<A>;
  using B2 = mpl::decay_t<B>;
  static constexpr bool value =
    (std::is_same<A2, B2>::value || std::is_same<mpl::remove_const_t<mpl::remove_pointer_t<A2>>,
                                                 mpl::remove_pointer_t<B2>>::value)
    && std::is_constructible<A, B>::value;
};

// If one is numeric and the other is pointer, after decay, it is forbidden.
template <typename A, typename B>
struct safely_constructible<A, B, mpl::enable_if_t<(is_numeric<A>::value && is_ptr<B>::value)
                                                   || (is_ptr<A>::value && is_numeric<B>::value)>> {
  static constexpr bool value = false;
};

} // end namespace strict_variant
