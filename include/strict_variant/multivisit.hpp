//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/mpl/tuple_cdr.hpp>
#include <strict_variant/mpl/typelist.hpp>
#include <strict_variant/mpl/ulist.hpp>
#include <strict_variant/variant.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

namespace strict_variant {
namespace mpl {

template <typename Visitor, typename... Args, unsigned... us>
auto
eval_visitor_impl(Visitor && vis, std::tuple<Args...> && tup, mpl::ulist<us...>) -> decltype(
  std::forward<Visitor>(std::declval<Visitor>())(std::forward<Args>(std::declval<Args>())...)) {
  return std::forward<Visitor>(vis)(std::forward<Args>(std::get<us>(tup))...);
}

// Tracks the state of how many things we have visited

template <typename Visitor, typename VisitedList, typename UnvisitedList>
struct multivisitor_state_base;

template <typename Visitor, typename... Vs, typename... Us>
struct multivisitor_state_base<Visitor, TypeList<Vs...>, TypeList<Us...>> {
  Visitor vis_;
  std::tuple<Vs...> vs_;
  std::tuple<Us...> us_;

  explicit multivisitor_state_base(Visitor vis, std::tuple<Vs...> vs, std::tuple<Us...> us)
    : vis_(std::forward<Visitor>(vis))
    , vs_(std::move(vs))
    , us_(std::move(us)) {}
};

// Primary template for multivisitor_state
template <typename Visitor, typename VisitedList, typename UnvisitedList>
struct multivisitor_state;

// When there is nothing left to visit then actually apply the visitor
// Need to use eval_visitor_impl to actually get the index sequence passed forward
template <typename Visitor, typename... Vs>
struct multivisitor_state<Visitor, TypeList<Vs...>, TypeList<>>
  : multivisitor_state_base<Visitor, TypeList<Vs...>, TypeList<>> {

  using base_t = multivisitor_state_base<Visitor, TypeList<Vs...>, TypeList<>>;

  // Ctor
  explicit multivisitor_state(Visitor vis, std::tuple<Vs...> vs, std::tuple<> us)
    : base_t(std::forward<Visitor>(vis), std::move(vs), std::move(us)) {}

  // Members
  using base_t::vis_;
  using base_t::vs_;
  using base_t::us_;

  decltype(std::forward<Visitor>(std::declval<Visitor>())(std::forward<Vs>(std::declval<Vs>())...))
  evaluate() {
    return eval_visitor_impl(std::forward<Visitor>(vis_), std::move(vs_),
                             mpl::count_t<sizeof...(Vs)>{});
  }
};

// Inductive hypothesis
// Clang really wants the functor to be a complete type at the time that we are evaluating
// "evaluate", so these need
// to be two separate types even if its more convoluted.
template <typename Visitor, typename VL, typename UL>
struct multivisitor_state_functor;

template <typename Visitor, typename... Vs, typename U1, typename... Us>
struct multivisitor_state_functor<Visitor, TypeList<Vs...>, TypeList<U1, Us...>>
  : multivisitor_state_base<Visitor, TypeList<Vs...>, TypeList<U1, Us...>> {

  using base_t = multivisitor_state_base<Visitor, TypeList<Vs...>, TypeList<U1, Us...>>;

  // Ctor
  explicit multivisitor_state_functor(Visitor vis, std::tuple<Vs...> vs, std::tuple<U1, Us...> us)
    : base_t(std::forward<Visitor>(vis), std::move(vs), std::move(us)) {}

  // Members
  using base_t::vis_;
  using base_t::vs_;
  using base_t::us_;

  template <typename T>
  using next_state_t = multivisitor_state<Visitor, TypeList<Vs..., T>, TypeList<Us...>>;

  // Update the state, by moving a value to the visited list and popping from Unvisited
  template <typename T>
  auto operator()(T && t) -> decltype(std::declval<next_state_t<T>>().evaluate()) {
    next_state_t<T> new_state{std::forward<Visitor>(vis_),
                              std::tuple_cat(vs_, std::tuple<T>{std::forward<T>(t)}),
                              mpl::cdr_tuple(us_)};
    return new_state.evaluate();
  }
};

template <typename Visitor, typename... Vs, typename U1, typename... Us>
struct multivisitor_state<Visitor, TypeList<Vs...>, TypeList<U1, Us...>>
  : multivisitor_state_functor<Visitor, TypeList<Vs...>, TypeList<U1, Us...>> {

  using functor_t = multivisitor_state_functor<Visitor, TypeList<Vs...>, TypeList<U1, Us...>>;

  // Ctor
  explicit multivisitor_state(Visitor vis, std::tuple<Vs...> vs, std::tuple<U1, Us...> us)
    : functor_t(std::forward<Visitor>(vis), std::move(vs), std::move(us)) {}

  using functor_t::us_;

  auto evaluate() -> decltype(apply_visitor(*static_cast<functor_t *>(nullptr),
                                            std::forward<U1>(std::declval<U1>()))) {
    return apply_visitor(*this, std::forward<U1>(std::get<0>(us_)));
  }
};

} // end namespace mpl

namespace detail {

template <typename Visitor, typename... Us>
auto
multivisit_impl(Visitor && visitor, Us &&... us) -> decltype(
  static_cast<mpl::multivisitor_state<Visitor, mpl::TypeList<>, mpl::TypeList<Us...>> *>(nullptr)
    ->evaluate()) {
  mpl::multivisitor_state<Visitor, mpl::TypeList<>, mpl::TypeList<Us...>> st{
    std::forward<Visitor>(visitor), {}, std::tuple<Us...>{std::forward<Us>(us)...}};
  return st.evaluate();
}

} // end namespace detail

template <typename Visitor, typename V1, typename V2, typename... Vs>
auto
apply_visitor(Visitor && vis, V1 && v1, V2 && v2, Vs &&... vs)
  -> decltype(detail::multivisit_impl(std::forward<Visitor>(std::declval<Visitor>()),
                                      std::forward<V1>(std::declval<V1>()),
                                      std::forward<V2>(std::declval<V2>()),
                                      std::forward<Vs>(std::declval<Vs>())...)) {
  return detail::multivisit_impl(std::forward<Visitor>(vis), std::forward<V1>(v1),
                                 std::forward<V2>(v2), std::forward<Vs>(vs)...);
}

} // end namespace strict_variant
