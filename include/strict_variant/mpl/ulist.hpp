//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>
#include <strict_variant/mpl/typelist.hpp>

namespace strict_variant {
namespace mpl {

template <unsigned... us>
struct ulist {
  static constexpr std::size_t size = sizeof...(us);
};

// Concat

template <unsigned... ul, unsigned... ur>
struct Concat<ulist<ul...>, ulist<ur...>> {
  typedef ulist<ul..., ur...> type;
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
template <std::size_t n>
struct count {
  using type = append_t<typename count<n - 1>::type, n - 1>;
};

template <>
struct count<0> {
  using type = ulist<>;
};

template <std::size_t n>
using count_t = typename count<n>::type;

// map function
template <template <unsigned> class F, typename TL>
struct ulist_map;

template <template <unsigned> class F, unsigned... us>
struct ulist_map<F, ulist<us...>> {
  using type = mpl::TypeList<typename F<us>::type...>;
};

template <template <unsigned> class F, typename TL>
using ulist_map_t = typename ulist_map<F, TL>::type;

// filter function

template <template <unsigned> class F, typename UL>
struct ulist_filter;

template <template <unsigned> class F>
struct ulist_filter<F, ulist<>> {
  using type = ulist<>;
};

template <template <unsigned> class F, unsigned u, unsigned... us>
struct ulist_filter<F, ulist<u, us...>> {
  using recurse_t = typename ulist_filter<F, ulist<us...>>::type;

  using type =
    typename std::conditional<F<u>::value, Concat_t<ulist<u>, recurse_t>, recurse_t>::type;
};

template <template <unsigned> class F, typename UL>
using ulist_filter_t = typename ulist_filter<F, UL>::type;

} // end namespace mpl
} // end namespace strict_variant
