//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <safe_variant/find_with.hpp>
#include <safe_variant/is_member_property.hpp>
#include <safe_variant/safely_constructible.hpp>
#include <safe_variant/typelist.hpp>
#include <safe_variant/variant_fwd.hpp>

#include <type_traits>
#include <utility>

namespace safe_variant {
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
/// Reinteprets' the "storage" pointer as a value of type T or const T,
/// then applies the visitor object.
template <typename Internal, typename T, typename Storage, typename Visitor, typename... Args>
auto
visitor_caller(Internal && internal, Storage && storage, Visitor && visitor, Args &&... args) ->
  typename mpl::remove_reference_t<Visitor>::result_type
 {
  typedef typename std::conditional<std::is_const<typename std::remove_extent<
                                      typename std::remove_reference<Storage>::type>::type>::value,
                                    const T, T>::type ConstType;

  return std::forward<Visitor>(visitor)(get_value(*reinterpret_cast<ConstType *>(storage), internal),
                 std::forward<Args>(args)...);
}

/// Helper object which dispatches a visitor object to the appropriate
/// interpretation of our
/// storage value, based on value of "which".
/// The solution here is that for each possible value of `which`, we create an
/// appropriate
/// function using the above function template, and store them each in an array,
/// using
/// parameter pack expansion. (A little 'jump table'.)
/// Then we dereference the array at index `m_which` and call that function.
/// This means we pick out the right function very quickly, but it may not be
/// inlined by the
/// compiler even if it is small.
template <typename... AllTypes>
struct jumptable_dispatch {
  template <typename Internal, typename VoidPtrCV, typename Visitor, typename... Args>
  auto operator()(Internal && internal,
                  const unsigned int which,
                  VoidPtrCV && storage,
                  Visitor && visitor,
                  Args &&... args) -> typename mpl::remove_reference_t<Visitor>::result_type {
    using whichCaller = typename mpl::remove_reference_t<Visitor>::result_type (*)(Internal &&, VoidPtrCV &&, Visitor &&,
                                                         Args && ...);

    static whichCaller callers[sizeof...(AllTypes)] = {
      &visitor_caller<Internal &&, AllTypes, VoidPtrCV &&, Visitor, Args &&...>...};

    // ASSERT(which < static_cast<unsigned int>(sizeof...(AllTypes)));

    return (*callers[which])(std::forward<Internal>(internal), std::forward<VoidPtrCV>(storage),
                             std::forward<Visitor>(visitor), std::forward<Args>(args)...);
  }
};

/// Same as the above, but we use a different strategy based on a binary tree,
/// and repeated testing
/// of the "which" value.
/// The idea is that if there are multiple types, we just test if we are looking
/// in the first half
/// or
/// the second half, and branch to two different instantiations of the
/// binary_search_dispatch object
/// as appropriate.
/// When arranged this way, the compiler can always inline the visitor calls,
/// and so for variants
/// with few types this may be significantly faster.
/// The "which" value is not changed even as the type list gets smaller,
/// instead, the "base" value
/// is
/// increased.
template <unsigned int, typename /*Type List */>
struct binary_search_dispatch;

template <unsigned int base, typename T>
struct binary_search_dispatch<base, TypeList<T>> {
  template <typename Internal, typename VoidPtrCV, typename Visitor, typename... Args>
  auto operator()(Internal && internal,
                  const unsigned int which,
                  VoidPtrCV && storage,
                  Visitor && visitor,
                  Args &&... args) -> typename mpl::remove_reference_t<Visitor>::result_type {
    // ASSERT(which == base);
    static_cast<void>(which);

    return visitor_caller<Internal &&, T, VoidPtrCV &&, Visitor, Args &&...>(
      std::forward<Internal>(internal), std::forward<VoidPtrCV>(storage),
      std::forward<Visitor>(visitor), std::forward<Args>(args)...);
  }
};

template <unsigned int base, typename T1, typename T2, typename... Types>
struct binary_search_dispatch<base, TypeList<T1, T2, Types...>> {
  typedef TypeList<T1, T2, Types...> TList;
  typedef Subdivide<TList> Subdiv;
  typedef typename Subdiv::L TL;
  typedef typename Subdiv::R TR;
  static constexpr unsigned int split_point = base + static_cast<unsigned int>(TL::size);

  template <typename Internal, typename VoidPtrCV, typename Visitor, typename... Args>
  auto operator()(Internal && internal,
                  const unsigned int which,
                  VoidPtrCV && storage,
                  Visitor && visitor,
                  Args &&... args) -> typename mpl::remove_reference_t<Visitor>::result_type {

    if (which < split_point) {
      return binary_search_dispatch<base, TL>{}(
        std::forward<Internal>(internal), which, std::forward<VoidPtrCV>(storage),
        std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    } else {
      return binary_search_dispatch<split_point, TR>{}(
        std::forward<Internal>(internal), which, std::forward<VoidPtrCV>(storage),
        std::forward<Visitor>(visitor), std::forward<Args>(args)...);
    }
  }
};

/// Choose the jumptable dispatch strategy when the number of types is > switch
/// point
/// choose the binary search dispatch for less than that.
/// Tentatively choosing 4 for switch point, potentially 8 is better... ?
/// Needs more rigorous benchmarking
template <typename... AllTypes>
struct visitor_dispatch {
  static constexpr unsigned int switch_point = 4;

  // using chosen_dispatch_t = jumptable_dispatch<AllTypes...>;

  using chosen_dispatch_t =
    typename std::conditional<(sizeof...(AllTypes) > switch_point),
                              jumptable_dispatch<AllTypes...>,
                              binary_search_dispatch<0, TypeList<AllTypes...>>>::type;

  template <typename Internal, typename VoidPtrCV, typename Visitor, typename... Args>
  auto operator()(Internal && internal,
                  const unsigned int which,
                  VoidPtrCV && storage,
                  Visitor && visitor,
                  Args &&... args) -> typename mpl::remove_reference_t<Visitor>::result_type {

    return chosen_dispatch_t{}(std::forward<Internal>(internal), which,
                               std::forward<VoidPtrCV>(storage), std::forward<Visitor>(visitor),
                               std::forward<Args>(args)...);
  }
};

/***
 * Metafunction `subvariant`: Check if the set of types of A is a subset of the
 * set of types of B
 */
template <typename A, typename B>
struct subvariant;

template <typename... As, typename... Bs>
struct subvariant<variant<As...>, variant<Bs...>> {
  static constexpr bool value = mpl::All_Have<mpl::is_member<Bs...>::template prop, As...>::value;
};

/***
 * Metafunction `proper_subvariant`:
 *   Check if both `sublist<A, B>::value` and `!std::is_same<A, B>::value` hold.
 * This is used to
 *   select when we should use "generalizing" ctor of variant, rather than one
 * of the usual special
 *   member functions. Note that it's possible that sizeof...(As) ==
 * sizeof...(Bs) but this doesn't
 *   cause any bugs so it's ok.
 */
template <typename A, typename B>
struct proper_subvariant {
  static constexpr bool value = !std::is_same<A, B>::value && subvariant<A, B>::value;
};

/***
 * Metafunction `allow_variant_construction`:
 *   Check if a type should be allowed to initalize our variant with the value
 *   of a second type. Basically, we always answer with
 * "mpl::safe_constructible<...>::value",
 *   unless one of them is recursively wrapped. If it is, then we only allow
 * copy ctor essentially,
 *   so that it can be an incomplete type.
 *   Note that we do some tricky stuff here to ensure that things can be
 * incomplete types,
 *   when using recursive wrapper.
 */
template <typename A, typename B, typename ENABLE = void>
struct allow_variant_construction;

template <typename A, typename B>
struct allow_variant_construction<A, B> {
  static constexpr bool value = mpl::safely_constructible<A, B>::value;
};

// By default, recursive wrapper construction is NOT allowed, unless expressly
// allowed below,
// via simple checks that don't require complete types.
template <typename T, typename U>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, U> {
  static constexpr bool value = false;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, T> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, const T &> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, T &&> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, safe_variant::recursive_wrapper<T>> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, const safe_variant::recursive_wrapper<T> &> {
  static constexpr bool value = true;
};

template <typename T>
struct allow_variant_construction<safe_variant::recursive_wrapper<T>, safe_variant::recursive_wrapper<T> &&> {
  static constexpr bool value = true;
};

/***
 * A template 'property' which uses the above
 */
template <typename T>
struct allow_variant_construct_from {
  template <typename U>
  struct prop {
    static constexpr bool value = allow_variant_construction<U, T>::value;
  };
};

/***
 * Self contained implementation of 'std::max' over a list of sizes...
 * by Jarryd Beck
 */
template <template <typename> class Size, typename SoFar, typename... Args>
struct max_helper;

template <template <typename> class Size, typename SoFar>
struct max_helper<Size, SoFar> {
  static constexpr decltype(Size<SoFar>::value) value = Size<SoFar>::value;
  typedef SoFar type;
};

template <template <typename> class Size, typename SoFar, typename Next, typename... Args>
struct max_helper<Size, SoFar, Next, Args...> {
private:
  typedef typename std::conditional<(Size<Next>::value > Size<SoFar>::value),
                                    max_helper<Size, Next, Args...>,
                                    max_helper<Size, SoFar, Args...>>::type m_next;

public:
  static constexpr decltype(Size<SoFar>::value) value = m_next::value;

  typedef typename m_next::type type;
};
} // end namespace detail

template <template <typename> class Size, typename... Args>
struct max;

template <template <typename> class Size, typename First, typename... Args>
struct max<Size, First, Args...> {
private:
  typedef decltype(Size<First>::value) m_size_type;
  typedef detail::max_helper<Size, First, Args...> m_helper;

public:
  static constexpr m_size_type value = m_helper::value;
  typedef typename m_helper::type type;
};

} // end namespace util
