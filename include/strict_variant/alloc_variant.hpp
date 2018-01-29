//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/alloc_wrapper.hpp>
#include <strict_variant/mpl/std_traits.hpp>
#include <strict_variant/variant.hpp>
#include <type_traits>

namespace strict_variant {

//[ strict_variant_alloc_variant
template <template <typename> class Alloc>
struct alloc_variant {

  // Version of wrap_if_throwing_move which uses alloc_wrapper<_, Alloc>

  template <typename T, typename = mpl::enable_if_t<std::is_nothrow_destructible<T>::value
                                                    && !std::is_reference<T>::value>>
  struct wrap_throwing_move {
    using type = typename std::conditional<std::is_nothrow_move_constructible<T>::value, T,
                                           alloc_wrapper<T, Alloc<T>>>::type;
  };

  template <typename T>
  using wrap_throwing_move_t = typename wrap_throwing_move<T>::type;

  template <typename... Ts>
  using type = variant<wrap_throwing_move_t<Ts>...>;
};
//]

} // end namespace strict_variant
