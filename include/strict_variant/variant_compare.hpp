//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional>
#include <strict_variant/variant.hpp>
#include <type_traits>

/***
 * This variant comparator allows comparing variants which are over
 * the same set of types, assuming each type involved is ordered
 * already.
 * The idea is that first we take "which" of both variants, and if
 * one is less than the other, then that is less overall. Otherwise,
 * get them both as the same type and use `<` comparator.
 *
 * This effectively creates a total ordering in which the orderings of
 * each type individually are concatenated.
 *
 * An alternative construction might allow these ranges to intermixed.
 *
 * This template splits the ordering problem into smaller parts --
 * to compare two variants, we only need to know how to compare elements
 * of each individual type, and how to compare ints. Both parts of this
 * problem use `std::less` by default but this may be modified using
 * template template parameters.
 *
 * An alternate name for this comparator might be "partition comparator",
 * since it exploits the natural partition of possible variant values.
 */

namespace strict_variant {

template <typename T, template <typename> class ComparatorTemplate = std::less,
          typename WhichComparator_t = std::less<int>>
struct variant_comparator;

template <typename... types, template <typename> class ComparatorTemplate,
          typename WhichComparator_t>
struct variant_comparator<variant<types...>, ComparatorTemplate, WhichComparator_t> {

  typedef variant<types...> var_t;

  struct helper {

    const var_t & first;
    const var_t & other;

    explicit helper(const var_t & _f, const var_t & _o)
      : first(_f)
      , other(_o) {}

    template <typename T>
    bool operator()(const T & t) const {
      if (const T * o = strict_variant::get<T>(&other)) {
        ComparatorTemplate<T> c; // make comparator
        return c(t, *o);
      } else {
        static_assert(std::is_same<int, decltype(first.which())>::value,
                      "The return type of 'variant::which' was changed and "
                      "variant_compare was not updated");
        WhichComparator_t c;
        return c(first.which(), other.which());
      }
    }
  };

  bool operator()(const var_t & v1, const var_t & v2) const {
    helper h{v1, v2};
    return strict_variant::apply_visitor(h, v1);
  }
};

} // end namespace strict_variant
