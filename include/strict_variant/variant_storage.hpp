//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <new>
#include <strict_variant/mpl/max.hpp>
#include <strict_variant/mpl/typelist.hpp>
#include <strict_variant/wrapper.hpp>
#include <utility>

namespace strict_variant {
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

  // alignas(m_align) char m_storage[m_size];
  using aligned_storage_t = typename std::aligned_storage<m_size, m_align>::type;
  aligned_storage_t m_storage;

  void * address() { return reinterpret_cast<void *>(&m_storage); }
  const void * address() const { return reinterpret_cast<const void *>(&m_storage); }

  /***
   * Index -> Type
   */
  using my_types = mpl::TypeList<First, Types...>;

  template <size_t index>
  using value_t = mpl::Index_At<my_types, index>;

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
   * "Internal" (non-piercing) access is achieved if detail::true_ is passed
   */
  template <size_t index>
  value_t<index> & get_value(detail::true_) & {
    return *reinterpret_cast<value_t<index> *>(this->address());
  }

  template <size_t index>
  const value_t<index> & get_value(detail::true_) const & {
    return *reinterpret_cast<const value_t<index> *>(this->address());
  }

  template <size_t index>
  value_t<index> && get_value(detail::true_) && {
    return std::move(*reinterpret_cast<value_t<index> *>(this->address()));
  }

  template <size_t index>
  unwrap_type_t<value_t<index>> & get_value(detail::false_) & {
    return detail::pierce_wrapper(this->get_value<index>(detail::true_{}));
  }

  template <size_t index>
  const unwrap_type_t<value_t<index>> & get_value(detail::false_) const & {
    return detail::pierce_wrapper(this->get_value<index>(detail::true_{}));
  }

  template <size_t index>
  unwrap_type_t<value_t<index>> && get_value(detail::false_) && {
    return std::move(detail::pierce_wrapper(this->get_value<index>(detail::true_{})));
  }
};

} // end namespace detail
} // end namespace strict_variant
