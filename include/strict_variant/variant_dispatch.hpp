//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/mpl/find_with.hpp>
#include <strict_variant/mpl/nonstd_traits.hpp>
#include <strict_variant/mpl/std_traits.hpp>
#include <strict_variant/mpl/typelist.hpp>
#include <strict_variant/mpl/ulist.hpp>
#include <strict_variant/variant_fwd.hpp>

#include <type_traits>
#include <utility>

#ifdef STRICT_VARIANT_DEBUG
#include <cassert>

#define STRICT_VARIANT_ASSERT(X)                                                                   \
  do {                                                                                             \
    assert((X));                                                                                   \
  } while (0)

#else // STRICT_VARIANT_DEBUG

#define STRICT_VARIANT_ASSERT(X)                                                                   \
  do {                                                                                             \
    static_cast<void>(X);                                                                          \
  } while (0)

#endif // STRICT_VARIANT_DEBUG

namespace strict_variant {

namespace detail {

/***
 * Dispatch mechanism:
 *   These two templates are used to dispatch visitor objects to
 *   the variant's storage. The problem that is being solved here is,
 *   how do we actually recover type information from the runtime
 *   value of `m_which`, so that we can call the appropriate overload
 *   of a visitor to the variant.
 */

/// Function which evaluates a visitor against a variant's internals
/// Reinteprets the "storage" as a value of type T or const T,
/// then applies the visitor object.
#define RESULT_EXPR                                                                                \
  std::forward<Visitor>(v)(std::forward<Storage>(s).template get_value<index>(Internal()))

template <unsigned index, typename Internal, typename Storage, typename Visitor>
auto
visitor_caller(Storage && s, Visitor && v) noexcept(noexcept(RESULT_EXPR))
  -> decltype(RESULT_EXPR) {
  return RESULT_EXPR;
}

#undef RESULT_EXPR

/// Helper which figures out the return type of multiple visitor calls
/// It's better for this to be separate of the dispatch mechanism, because
/// std::common_type can technically be order dependent. It's confusing if the
/// return type can change depending on the dispatch strategy used, and it
/// simplifies the dispatch code to only have to implement this once.
template <typename Internal, typename Storage, typename Visitor>
struct return_typer {

#define RESULT_EXPR                                                                                \
  visitor_caller<index, Internal, Storage>(std::forward<Storage>(std::declval<Storage>()),         \
                                           std::forward<Visitor>(std::declval<Visitor>()))

  template <unsigned index>
  struct at_index {
    using type = decltype(RESULT_EXPR);
  };

  template <unsigned index>
  struct noexcept_prop {
    static constexpr bool value = noexcept(RESULT_EXPR);
    using type = std::integral_constant<bool, value>;
  };

#undef RESULT_EXPR
};

/// Helper object which dispatches a visitor object to the appropriate
/// interpretation of our storage value, based on value of "which".
///
/// The solution here is that for each possible value of `which`, we create an
/// appropriate function using the above function template, and store them each
/// in an array, using parameter pack expansion. (A little 'jump table'.)
///
/// Then we dereference the array at index `m_which` and call that function.
/// This means we pick out the right function very quickly, but it may not be
/// inlined by the compiler even if it is small.

template <typename return_t, typename Internal, typename ulist>
struct jumptable_dispatch;

template <typename return_t, typename Internal, unsigned... Indices>
struct jumptable_dispatch<return_t, Internal, mpl::ulist<Indices...>> {
  template <typename Storage, typename Visitor>
  return_t operator()(const unsigned int which, Storage && storage, Visitor && visitor)

  {
    using whichCaller = return_t (*)(Storage, Visitor);

    static whichCaller callers[sizeof...(Indices)] = {
      &visitor_caller<Indices, Internal, Storage, Visitor>...};

    STRICT_VARIANT_ASSERT(which < static_cast<unsigned int>(sizeof...(Indices)));

    return (*callers[which])(std::forward<Storage>(storage), std::forward<Visitor>(visitor));
  }
};

/// Same as the above, but we use a different strategy based on a binary tree,
/// and repeated testing of the "which" value.
///
/// The idea is that if there are multiple types, we just test if we are looking
/// in the first half or the second half, and branch to two different
/// instantiations of the binary_search_dispatch object as appropriate.
///
/// When arranged this way, the compiler can always inline the visitor calls,
/// and so for variants with few types this may be significantly faster.
///
/// The "which" value is not changed even as the type list gets smaller,
/// instead, the "base" value is increased.

template <typename return_t, typename Internal, unsigned int base, unsigned int num_types>
struct binary_search_dispatch {
  static_assert(num_types >= 2, "Something wrong with binary search dispatch");

  static constexpr unsigned int half = num_types / 2;

  template <typename Storage, typename Visitor>
  return_t operator()(const unsigned int which, Storage && storage, Visitor && visitor) {

    if (which < base + half) {
      return binary_search_dispatch<return_t, Internal, base, half>{}(
        which, std::forward<Storage>(storage), std::forward<Visitor>(visitor));
    } else {
      return binary_search_dispatch<return_t, Internal, base + half, num_types - half>{}(
        which, std::forward<Storage>(storage), std::forward<Visitor>(visitor));
    }
  }
};

template <typename return_t, typename Internal, unsigned int base>
struct binary_search_dispatch<return_t, Internal, base, 1u> {
  template <typename Storage, typename Visitor>
  return_t operator()(const unsigned int which, Storage && storage, Visitor && visitor) {
    STRICT_VARIANT_ASSERT(which == base);

    return visitor_caller<base, Internal, Storage, Visitor>(std::forward<Storage>(storage),
                                                            std::forward<Visitor>(visitor));
  }
};

/// Choose the jumptable dispatch strategy when the number of types is > switch
/// point
/// choose the binary search dispatch for less than that.
/// Tentatively choosing 4 for switch point, potentially 8 is better... ?
/// Needs more rigorous benchmarking
template <typename Internal, size_t num_types>
struct visitor_dispatch {
  // static constexpr unsigned int switch_point = 4;

  // Helper which takes the conjunction of a typelist of `std::integral_constant<bool>`.
  template <typename T>
  struct conjunction;

  template <typename... Types>
  struct conjunction<mpl::TypeList<Types...>> {
    template <typename T>
    struct is_false_prop : std::is_same<T, std::integral_constant<bool, false>> {};

    static constexpr bool value = !mpl::Find_Any<is_false_prop, Types...>::value;
  };

  // Helper which figures out return type and noexcept status, for given storage and visitor
  template <typename Storage, typename Visitor>
  struct call_helper {
    using rtyper = return_typer<Internal, Storage, Visitor>;
    using indices = mpl::count_t<num_types>;

    static constexpr bool noexcept_value =
      conjunction<mpl::ulist_map_t<rtyper::template noexcept_prop, indices>>::value;

    using return_type =
      typename mpl::typelist_fwd<mpl::common_return_type_t,
                                 mpl::ulist_map_t<rtyper::template at_index, indices>>::type;
  };

  // Invoke the actual dispatcher
  template <typename Storage, typename Visitor>
  auto operator()(const unsigned int which, Storage && storage,
                  Visitor && visitor) noexcept(call_helper<Storage, Visitor>::noexcept_value) ->
    typename call_helper<Storage, Visitor>::return_type {

    using return_t = typename call_helper<Storage, Visitor>::return_type;

    // using chosen_dispatch_t = jumptable_dispatch<return_t, Internal, mpl::count_t<num_types>>;

    // using chosen_dispatch_t =
    //  typename std::conditional<(num_types > switch_point),
    //  jumptable_dispatch<return_t, Internal,  mpl::count_t<num_types>>,
    //                            binary_search_dispatch<return_t, Internal, 0,
    //                             num_types>>::type;

    using chosen_dispatch_t = binary_search_dispatch<return_t, Internal, 0, num_types>;

    return chosen_dispatch_t{}(which, std::forward<Storage>(storage),
                               std::forward<Visitor>(visitor));
  }
};

} // end namespace detail

} // end namespace strict_variant

#undef STRICT_VARIANT_ASSERT
