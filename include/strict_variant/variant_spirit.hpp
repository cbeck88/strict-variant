//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/mpl/find_with.hpp>
#include <strict_variant/variant_fwd.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/spirit/home/support.hpp>
#include <type_traits>

/***
 * Add a trait within boost spirit so that strict_variant can be parsed using the
 * alternative parser operator |, just like boost::variant.
 */

// First a helper metafunction
namespace strict_variant {
namespace mpl {

// index_at_or_fallback
template <std::size_t n, typename fallback, typename... Types>
struct Index_At_Or_Fallback;

template <std::size_t n, typename F, typename T, typename... Ts>
struct Index_At_Or_Fallback<n, F, T, Ts...> : Index_At_Or_Fallback<n - 1, F, Ts...> {};

template <typename F, typename T, typename... Ts>
struct Index_At_Or_Fallback<0, F, T, Ts...> {
  typedef T type;
};

template <std::size_t n, typename F>
struct Index_At_Or_Fallback<n, F> {
  typedef F type;
};

} // end namespace mpl
} // end namespace strict_variant

// Now the qi parts
namespace boost {
namespace spirit {
namespace traits {

// forward declare the trait
template <typename T, typename Domain, typename Enable /* = void*/>
struct not_is_variant;

// mpl::false_ here is why we need to include boost mpl
template <typename First, typename... Types, typename Domain>
struct not_is_variant<strict_variant::variant<First, Types...>, Domain, void> : mpl::false_ {};

} // end namespace traits

// Add implementation for "find_substitute"

namespace qi {
namespace detail {

// declaration
template <typename Variant, typename Expected>
struct find_substitute;

template <typename... Types, typename Expected>
struct find_substitute<strict_variant::variant<Types...>, Expected> {
  template <typename T>
  struct same_prop : std::is_same<T, Expected> {};

  static constexpr size_t same_idx = strict_variant::mpl::Find_With<same_prop, Types...>::value;

  template <typename T>
  struct conv_prop : traits::is_substitute<T, Expected> {};

  static constexpr size_t same_or_conv_idx =
    (same_idx < sizeof...(Types)) ? same_idx
                                  : strict_variant::mpl::Find_With<conv_prop, Types...>::value;

  typedef typename strict_variant::mpl::Index_At_Or_Fallback<same_or_conv_idx, Expected,
                                                             Types...>::type type;
};

} // end namespace detail
} // end namespace qi

} // end namespace spirit
} // end namespace boost
