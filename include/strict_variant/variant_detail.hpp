//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/mpl/find_with.hpp>
#include <strict_variant/recursive_wrapper.hpp>
#include <strict_variant/safely_constructible.hpp>
#include <strict_variant/variant_fwd.hpp>

#include <type_traits>

/***
 * Traits to help with SFINAE in variant class template
 */

namespace strict_variant {

namespace detail {

// Search prediate for find_which
// Checks if type T and type U are the same, modulo const and recursive wrapper
template <typename T>
struct same_modulo_const_ref_wrapper {
  template <typename U>
  struct prop {
    using T2 = unwrap_type_t<mpl::remove_const_t<mpl::remove_reference_t<T>>>;
    using U2 = unwrap_type_t<mpl::remove_const_t<mpl::remove_reference_t<U>>>;

    static constexpr bool value = std::is_same<T2, U2>::value;
  };
};

/***
 * Property `is_member_modulo_const_ref_wrappr` checks if an element is in a
 * list, modulo const and recursive wrapper.
 */
template <typename... Ts>
struct is_member_modulo_const_ref_wrapper {
  template <typename U>
  struct prop {
    static constexpr bool value =
      mpl::Find_Any<same_modulo_const_ref_wrapper<U>::template prop, Ts...>::value;
  };
};

/***
 * Metafunction `subvariant`: Check if the set of types of A is a subset of the
 * set of types of B, modulo const and reference_wrapper
 */
template <typename A, typename B>
struct subvariant;

template <typename... As, typename... Bs>
struct subvariant<variant<As...>, variant<Bs...>> {
  static constexpr bool value =
    mpl::All_Have<is_member_modulo_const_ref_wrapper<Bs...>::template prop, As...>::value;
};

/***
 * Metafunction `proper_subvariant`:
 *   Check both `subvariant<A, B>::value` and `!std::is_same<A, B>::value`.
 *   This is used to select when we should use "generalizing" ctor of variant,
 *   rather than one of the usual special member functions.
 *
 *   Note that it's possible that sizeof...(As) == sizeof...(Bs) but this
 *   doesn't cause any bugs so it's ok -- it just means that
 *   `variant<int, double>` is constructible from `variant<double, int>`, which
 *   is fine.
 */
template <typename A, typename B>
struct proper_subvariant {
  static constexpr bool value = !std::is_same<A, B>::value && subvariant<A, B>::value;
};

/***
 * Metafunction `allow_variant_construction`:
 *   Check if a type should be allowed to initalize our variant with the value
 *   of a second type. Basically, we always answer with
 *   "mpl::safe_constructible<...>::value",
 *   unless one of them is recursively wrapped. If it is, then we only allow
 *   copy ctor essentially, so that it can be an incomplete type.
 *   Note that we do some tricky stuff here to ensure that things can be
 *   incomplete types, when using recursive wrapper.
 */
template <typename A, typename B>
struct allow_variant_construction : mpl::safely_constructible<A, B> {};

// By default, recursive wrapper construction is NOT allowed, unless expressly
// allowed below,
// via simple checks that don't require complete types.

// Usually we want to unwrap recursive_wrapper when we perform the check.
// If we are being assigned *from* a recursive wrapper, it means we are
template <typename T, typename U>
struct allow_variant_construction<strict_variant::recursive_wrapper<T>, U>
  : mpl::safely_constructible<T, U> {};

/*
template <typename T, typename U>
struct allow_variant_construction<strict_variant::recursive_wrapper<T>,
strict_variant::recursive_wrapper<U>> :
  mpl::safely_constructible<T, U> {};

template <typename T, typename U>
struct allow_variant_construction<strict_variant::recursive_wrapper<T>,
strict_variant::recursive_wrapper<U> &> :
  mpl::safely_constructible<T, U &> {};

template <typename T, typename U>
struct allow_variant_construction<strict_variant::recursive_wrapper<T>,
strict_variant::recursive_wrapper<U> &&> :
  mpl::safely_constructible<T, U &&> {};

template <typename T, typename U>
struct allow_variant_construction<strict_variant::recursive_wrapper<T>,
strict_variant::recursive_wrapper<U> const &> :
  mpl::safely_constructible<T, U const &> {};

template <typename T, typename U>
struct allow_variant_construction<strict_variant::recursive_wrapper<T>,
strict_variant::recursive_wrapper<T>> : {
  static constexpr value = true;
  static constexpr priority = mpl::priority<5>;
};

template <typename T, typename U>
struct allow_variant_construction<strict_variant::recursive_wrapper<T>,
strict_variant::recursive_wrapper<T> &&> : {
  static constexpr value = true;
  static constexpr priority = mpl::priority<5>;
};
*/

/****
 * NOEXCEPT TRAITS
 *
 * Traits to help with moveability / copyability in variant.
 * The trick is that we need to support the case when recursive_wrapper is incomplete.
 */

template <typename T>
struct is_nothrow_moveable : std::is_nothrow_move_constructible<T> {};

template <typename T>
struct is_nothrow_moveable<recursive_wrapper<T>> : std::true_type {};

template <typename T>
struct is_nothrow_copyable : std::is_nothrow_constructible<T, const T &> {};

template <typename T> // Just assume it isn't, since it may be incomplete
struct is_nothrow_copyable<recursive_wrapper<T>> : std::false_type {};

template <typename T>
struct is_nothrow_move_assignable : std::is_nothrow_move_assignable<T> {};

template <typename T>
struct is_nothrow_move_assignable<recursive_wrapper<T>> : std::true_type {};

template <typename T>
struct is_nothrow_copy_assignable : std::is_nothrow_copy_assignable<T> {};

template <typename T> // Just assume it isn't, since it may be incomplete
struct is_nothrow_copy_assignable<recursive_wrapper<T>> : std::false_type {};

template <typename First, typename... Types>
struct variant_noexcept_helper {
  /***
   * Configuration
   */

  // Treat all input types as if they were nothrow moveable,
  // regardless of their noexcept declarations.
  // (Note: It is a core assumption of the variant that these operations don't
  //  throw, you will get UB and likely an immediate crash if they do throw.
  //  Only use this as a workaround for e.g. old code which is not
  //  noexcept-correct. For instance if you require compatibility with a
  //  gcc-4-series version of libstdc++ and std::string isn't noexcept.)
  static constexpr bool assume_move_nothrow =
#ifdef STRICT_VARIANT_ASSUME_MOVE_NOTHROW
    true;
#else
    false;
#endif

  // Treat all input types as if they were nothrow copyable,
  // regardless of thier noexcept declarations.
  // (Note: This is usually quite risky, only appropriate in projects which
  //  assume already that dynamic memory allocation will never fail, and want to
  //  go as fast as possible given that assumption.)
  static constexpr bool assume_copy_nothrow =
#ifdef STRICT_VARIANT_ASSUME_COPY_NOTHROW
    true;
#else
    false;
#endif

  static constexpr bool nothrow_move_ctors =
    assume_move_nothrow || mpl::All_Have<detail::is_nothrow_moveable, First, Types...>::value;

  static constexpr bool nothrow_copy_ctors =
    assume_copy_nothrow || mpl::All_Have<detail::is_nothrow_copyable, First, Types...>::value;

  static constexpr bool nothrow_move_assign =
    nothrow_move_ctors && mpl::All_Have<detail::is_nothrow_move_assignable, First, Types...>::value;

  static constexpr bool nothrow_copy_assign =
    nothrow_copy_ctors && mpl::All_Have<detail::is_nothrow_copy_assignable, First, Types...>::value;
};

} // end namespace detail

} // end namespace strict_variant
