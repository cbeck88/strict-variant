//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace strict_variant {
namespace mpl {

template <unsigned... us>
struct ulist {
  static constexpr size_t size = sizeof...(us);
};

// Append
template <typename UL, unsigned u>
struct append;

template <unsigned u, unsigned... us>
struct append<ulist<us...>, u> {
  using type = ulist<us..., u>;
};

template <typename UL, unsigned u>
using append_t = typename append<UL, u>::type;

// Count function
template <size_t n>
struct count {
  using type = append_t<typename count<n - 1>::type, n - 1>;
};

template <>
struct count<0> {
  using type = ulist<>;
};

template <size_t n>
using count_t = typename count<n>::type;

// map function
template <template <unsigned> class F, typename TL>
struct ulist_map;

template <template <unsigned> class F, unsigned... us>
struct ulist_map<F, ulist<us...>> {
  using type = TypeList<typename F<us>::type...>;
};

} // end namespace mpl
} // end namespace strict_variant
