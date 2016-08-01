//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>
#include <safe_variant/mpl/index.hpp>
#include <safe_variant/mpl/max.hpp>
#include <safe_variant/recursive_wrapper.hpp>
#include <utility>

namespace safe_variant {
namespace detail {

// Implementation note:
// Internal visitors need to be able to access the "true" type, the
// reference_wrapper<T> when it is within the variant, to implement ctors
// and special memeber functions.
// External visitors are supposed to have this elided.
// The `get_value` function uses tag dispatch to do the right thing.

struct true_ {};
struct false_ {};

// Storage for the types in a list of types.
// Provides typed access using the index within the list as a template parameter.
// And some facilities for piercing recursive_wrapper

template <typename First, typename... Types>
struct storage {

  /***
   * Determine size and alignment of our storage
   */
  template <typename T>
  struct Sizeof {
    static constexpr size_t value = sizeof(T);
  };

  template <typename T>
  struct Alignof {
    static constexpr size_t value = alignof(T);
  };

  // size = max of size of each thing
  static constexpr size_t m_size = mpl::max<Sizeof, First, Types...>::value;

  // align = max align of each thing
  static constexpr size_t m_align = mpl::max<Alignof, First, Types...>::value;

  /***
   * Storage
   */

  alignas(m_align) char m_storage[m_size];

  void * address() { return m_storage; }
  const void * address() const { return m_storage; }

  /***
   * Index -> Type
   */
  template <size_t index>
  using value_t = mpl::Index_t<index, First, Types...>;

  /***
   * Initialize to the type at a particular value
   */
  template <size_t index, typename... Args>
  void initialize(Args &&... args) noexcept(
    noexcept(value_t<index>(std::forward<Args>(std::declval<Args>())...))) {
    new (this->address()) value_t<index>(std::forward<Args>(args)...);
  }

  /***
   * Typed access which pierces recursive_wrapper if detail::false_ is passed
   */
  template <size_t index, typename Internal>
  value_t<index> & get_value(const Internal &) & {
    return *reinterpret_cast<value_t<index> *>(this->address());
  }

  template <size_t index, typename Internal>
  const value_t<index> & get_value(const Internal &) const & {
    return *reinterpret_cast<const value_t<index> *>(this->address());
  }

  template <size_t index, typename Internal>
  value_t<index> && get_value(const Internal &) && {
    return std::move(*reinterpret_cast<value_t<index> *>(this->address()));
  }

  template <size_t index>
  unwrap_type_t<value_t<index>> & get_value(const detail::false_ &) & {
    return detail::pierce_recursive_wrapper(this->get_value<index>(detail::true_{}));
  }

  template <size_t index>
  const unwrap_type_t<value_t<index>> & get_value(const detail::false_ &) const & {
    return detail::pierce_recursive_wrapper(this->get_value<index>(detail::true_{}));
  }

  template <size_t index>
  unwrap_type_t<value_t<index>> && get_value(const detail::false_ &) && {
    return std::move(detail::pierce_recursive_wrapper(this->get_value<index>(detail::true_{})));
  }
};

} // end namespace detail
} // end namespace safe_variant
