//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

namespace strict_variant {
namespace mpl {

// Poor man's boost::mpl::vector.
template <class... Ts>
struct TypeList {
  static constexpr int size = sizeof...(Ts);
};

template <class List, class T>
struct Append;

template <class... Ts, class T>
struct Append<TypeList<Ts...>, T> {
  typedef TypeList<Ts..., T> type;
};

/***
 * Some metafunctions for working with typelists
 */

// Metafunction Concat: Concatenate two typelists
template <typename L, typename R>
struct Concat;

template <typename... TL, typename... TR>
struct Concat<TypeList<TL...>, TypeList<TR...>> {
  typedef TypeList<TL..., TR...> type;
};

template <typename L, typename R>
using Concat_t = typename Concat<L, R>::type;

// Metafunction Split: Split a typelist at a particular index
template <int i, typename TL>
struct Split;

template <int k, typename... TL>
struct Split<k, TypeList<TL...>> {
private:
  typedef Split<k / 2, TypeList<TL...>> FirstSplit;
  typedef Split<k - k / 2, typename FirstSplit::R> SecondSplit;

public:
  typedef Concat_t<typename FirstSplit::L, typename SecondSplit::L> L;
  typedef typename SecondSplit::R R;
};

template <typename T, typename... TL>
struct Split<0, TypeList<T, TL...>> {
  typedef TypeList<> L;
  typedef TypeList<T, TL...> R;
};

template <typename T, typename... TL>
struct Split<1, TypeList<T, TL...>> {
  typedef TypeList<T> L;
  typedef TypeList<TL...> R;
};

template <int k>
struct Split<k, TypeList<>> {
  typedef TypeList<> L;
  typedef TypeList<> R;
};

// Metafunction Subdivide: Split a typelist into two roughly equal typelists
template <typename TL>
struct Subdivide : Split<TL::size / 2, TL> {};

/***
 * Index_At metafunction
 * Based this on Subdivide, in order to reduce recursion depth
 */
template <typename T, std::size_t i, typename Enable = void>
struct Index_At_s;

// Prevent out of bounds
template <typename T>
struct Index_At_s<TypeList<T>, 0> {
  using type = T;
};

template <typename TL, std::size_t i>
struct Index_At_s<TL, i, typename std::enable_if<(TL::size >= 2 && i < TL::size / 2)>::type> {
  using type = typename Index_At_s<typename Subdivide<TL>::L, i>::type;
};

template <typename TL, std::size_t i>
struct Index_At_s<TL, i, typename std::enable_if<(TL::size >= 2 && TL::size / 2 <= i
                                                  && i < TL::size)>::type> {
  using type = typename Index_At_s<typename Subdivide<TL>::R, i - (TL::size / 2)>::type;
};

template <typename T, std::size_t i>
using Index_At = typename Index_At_s<T, i>::type;

/***
 * Back_Of metafunction
 */
template <typename T>
struct Back_Of_s;

template <typename T, typename... Ts>
struct Back_Of_s<TypeList<T, Ts...>> : Index_At<TypeList<T, Ts...>, sizeof...(Ts)> {};

template <typename T>
using Back_Of = Back_Of_s<T>;

/***
 * typelist_fwd metafunction
 * Pass the members of a typelist to a variadic template
 */
template <template <class...> class, typename TL>
struct typelist_fwd;

template <template <class...> class F, typename... Ts>
struct typelist_fwd<F, TypeList<Ts...>> {
  using type = F<Ts...>;
};

} // end namespace mpl
} // end namespace strict_variant
