//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/conversion_rank.hpp>
#include <strict_variant/mpl/std_traits.hpp>
#include <strict_variant/safe_arithmetic_conversion.hpp>
#include <strict_variant/safe_pointer_conversion.hpp>
#include <type_traits>

namespace strict_variant {

/***
 * Metafunction `safely_constructible`:
 *   A metafunction yielding a bool which detects is A is constructible from B without using
 *   conversions that we consider "unsafe".
 *
 *   The main purpose of this trait is to support `strict_variant::variant` in deciding what
 *   conversions to allow, and how to deduce what internal type to assign to any given type.
 */

//[ strict_variant_safely_constructible
// Helper metafunction which checks if type is arithmetic after decay.
// This is the same as, `is_arithmetic` modulo `const` and references.
template <typename T>
struct decay_to_arithmetic : std::is_arithmetic<mpl::decay_t<T>> {};

// Helper metafunction which checks if type is pointer after decay
template <typename T>
struct decay_to_ptr : std::is_pointer<mpl::decay_t<T>> {};

// Primary template, falls back to std::is_constructible
template <typename A, typename B, typename ENABLE = void>
struct safely_constructible : std::is_constructible<A, B> {};

// If both are arithmetic after decay, then pass to safe_arithmetic_conversion
template <typename A, typename B>
struct safely_constructible<A, B, mpl::enable_if_t<decay_to_arithmetic<A>::value
                                                   && decay_to_arithmetic<B>::value>>
  : safe_arithmetic_conversion<mpl::decay_t<A>, mpl::decay_t<B>> {};

// If both are pointer after decay, then pass to safe_pointer_conversion
template <typename A, typename B>
struct safely_constructible<A, B,
                            mpl::enable_if_t<decay_to_ptr<A>::value && decay_to_ptr<B>::value>>
  : safe_pointer_conversion<mpl::decay_t<A>, mpl::decay_t<B>> {};

// If one is arithmetic and the other is pointer, after decay, it is forbidden.
template <typename A, typename B>
struct safely_constructible<A, B, mpl::enable_if_t<(decay_to_arithmetic<A>::value
                                                    && decay_to_ptr<B>::value)
                                                   || (decay_to_ptr<A>::value
                                                       && decay_to_arithmetic<B>::value)>>
  : std::false_type {};
//]

} // end namespace strict_variant
