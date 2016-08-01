//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <safe_variant/mpl/find_with.hpp>
#include <safe_variant/mpl/is_member_property.hpp>
#include <safe_variant/recursive_wrapper.hpp>
#include <safe_variant/safely_constructible.hpp>
#include <safe_variant/variant_fwd.hpp>

#include <type_traits>

/***
 * Traits to help with SFINAE in variant class template
 */

namespace safe_variant {

namespace detail {

  // Search prediate for find_which
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
 * Metafunction `subvariant`: Check if the set of types of A is a subset of the
 * set of types of B
 */
template <typename A, typename B>
struct subvariant;

template <typename... As, typename... Bs>
struct subvariant<variant<As...>, variant<Bs...>> {
  static constexpr bool value = mpl::All_Have<mpl::is_member<Bs...>::template prop, As...>::value;
};

/***
 * Metafunction `proper_subvariant`:
 *   Check if both `sublist<A, B>::value` and `!std::is_same<A, B>::value` hold.
 * This is used to
 *   select when we should use "generalizing" ctor of variant, rather than one
 * of the usual special
 *   member functions. Note that it's possible that sizeof...(As) ==
 * sizeof...(Bs) but this doesn't
 *   cause any bugs so it's ok.
 */
template <typename A, typename B>
struct proper_subvariant {
  static constexpr bool value = !std::is_same<A, B>::value && subvariant<A, B>::value;
};

/***
 * Metafunction `allow_variant_construction`:
 *   Check if a type should be allowed to initalize our variant with the value
 *   of a second type. Basically, we always answer with
 * "mpl::safe_constructible<...>::value",
 *   unless one of them is recursively wrapped. If it is, then we only allow
 * copy ctor essentially,
 *   so that it can be an incomplete type.
 *   Note that we do some tricky stuff here to ensure that things can be
 * incomplete types,
 *   when using recursive wrapper.
 */
template <typename A, typename B, typename ENABLE = void>
struct allow_variant_construction;

template <typename A, typename B>
struct allow_variant_construction<A, B> {
  static constexpr bool value = mpl::safely_constructible<A, B>::value;
};

// By default, recursive wrapper construction is NOT allowed, unless expressly
// allowed below,
// via simple checks that don't require complete types.
template <typename T, typename U>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, U> {
  static constexpr bool value = false;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, T> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, const T &> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, T &&> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>,
                                  safe_variant::recursive_wrapper<T>> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>,
                                  const safe_variant::recursive_wrapper<T> &> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>,
                                  safe_variant::recursive_wrapper<T> &&> {
  static constexpr bool value = true;
};

/***
 * A template 'property' which uses the above
 */
template <typename T>
struct allow_variant_construct_from {
  template <typename U>
  struct prop {
    static constexpr bool value = allow_variant_construction<U, T>::value;
  };
};

} // end namespace detail

} // end namespace safe_variant
