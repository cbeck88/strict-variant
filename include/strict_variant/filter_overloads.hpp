//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/conversion_rank.hpp>
#include <strict_variant/mpl/find_any_in_list.hpp>
#include <strict_variant/mpl/typelist.hpp>
#include <strict_variant/mpl/ulist.hpp>
#include <strict_variant/recursive_wrapper.hpp>
#include <strict_variant/safely_constructible.hpp>

namespace strict_variant {

// Dominates is a condition that we use to eliminate overloads in some cases.
// If C is the argument, and A and B are in an overload set, and C is safely
// convertible to A and to B, and A dominates B, then B will be removed.
// Dominates should be thought of as a predicate on A and B, but is allowed to
// be sensitive to the identity of C.
//[ strict_variant_dominates_trait
template <typename A, typename B, typename C, typename ENABLE = void>
struct dominates : std::false_type {};

template <typename A, typename B, typename C>
struct dominates<A, B, C,
                 mpl::enable_if_t<decay_to_arithmetic<A>::value && decay_to_arithmetic<B>::value
                                  && decay_to_arithmetic<C>::value
                                  && !safely_constructible<A, B>::value
                                  && safely_constructible<B, A>::value>> : std::true_type {};
//]

// filter_overloads implements filtering rules for in variant(T&&) ctor
template <typename T, typename TL>
struct filter_overloads;

template <typename T, typename... Ts>
struct filter_overloads<T, mpl::TypeList<Ts...>> {
  using unwrapped_types = mpl::TypeList<unwrap_type_t<Ts>...>;

  template <unsigned u>
  struct get_t {
    using type = mpl::Index_At<unwrapped_types, u>;
  };

  template <unsigned u>
  struct safe_prop : safely_constructible<typename get_t<u>::type, T> {};

  using safe_pool = mpl::ulist_filter_t<safe_prop, mpl::count_t<sizeof...(Ts)>>;

  using safe_types = mpl::ulist_map_t<get_t, safe_pool>;

  template <typename U>
  struct dominating {
    template <typename V>
    struct prop : dominates<V, U, T> {};
  };

  template <unsigned u>
  struct undominated {
    static constexpr bool value =
      !mpl::Find_Any_In_List<dominating<typename get_t<u>::type>::template prop, safe_types>::value;
  };

  using safe_and_undominated = mpl::ulist_filter_t<undominated, safe_pool>;

  // result
  using type = safe_and_undominated;
};

} // end namespace strict_variant
