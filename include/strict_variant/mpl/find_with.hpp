//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef> // for std::size_t

namespace strict_variant {
namespace mpl {

/***
 * Some random template metafunction utilities, originally for variant
 */

// helper any_has, check a (boolean) property against a list. returns index of
// found item, or,
// sizeof + 1
template <template <typename> class Property, typename...>
struct Find_With;

template <template <typename> class Property>
struct Find_With<Property> {
  static constexpr std::size_t value = 1;
};

template <template <typename> class Property, typename T, typename... Types>
struct Find_With<Property, T, Types...> {
  static constexpr std::size_t value =
    (Property<T>::value ? 0 : (1 + Find_With<Property, Types...>::value));
};

// find_any
template <template <typename> class Property, typename... Ts>
struct Find_Any {
  static constexpr bool value = Find_With<Property, Ts...>::value < sizeof...(Ts);
};

// Negate_Property
template <template <typename> class Property>
struct Negate_Property {
  template <typename T>
  struct prop {
    static constexpr bool value = !Property<T>::value;
  };
};

// None_Have
template <template <typename> class Property, typename... Ts>
struct None_Have {
  static constexpr bool value = !Find_Any<Property, Ts...>::value;
};

// All_Have
template <template <typename> class Property, typename... Ts>
struct All_Have {
  static constexpr bool value = !Find_Any<Negate_Property<Property>::template prop, Ts...>::value;
};

} // end namespace mpl
} // end namespace strict_variant
