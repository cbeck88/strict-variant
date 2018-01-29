//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional>
#include <strict_variant/variant.hpp>
#include <type_traits>

/***
 * This variant comparator allows comparing variants which are over
 * the same set of types, assuming each type involved is less-than comparable
 * already.
 * The idea is that first we take "which" of both variants, and if
 * one is less than the other, then that variant is less overall. Otherwise,
 * get them both as the same type and compare those.
 *
 * This creates a total ordering on the variant values in which the orderings of
 * each value type sit side by side.
 *
 * The template requires a template template parameter, ComparatorTemplate,
 * which must have an implementation for each value type.
 * And it requires an `int` comparator to be used for the `which` values.
 * By default these are `std::less` and `std::less<int>`.
 */

//[ strict_variant_variant_comparator
namespace strict_variant {

template <typename T, template <typename> class ComparatorTemplate = std::less,
          typename WhichComparator_t = std::less<int>>
struct variant_comparator;
}
//]

namespace strict_variant {

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
        ComparatorTemplate<T> c;
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
