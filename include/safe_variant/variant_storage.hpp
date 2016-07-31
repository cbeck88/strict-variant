//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <safe_variant/mpl/index.hpp>
#include <safe_variant/mpl/max.hpp>
#include <utility>

namespace safe_variant {
namespace detail {

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
   * Unchecked typed access to storage
   */
  template <size_t index>
  mpl::Index_t<index, First, Types...> * unchecked_access() {
    return reinterpret_cast<mpl::Index_t<index, First, Types...> *>(this->address());
  }

  template <size_t index>
  const mpl::Index_t<index, First, Types...> * unchecked_access() const {
    return reinterpret_cast<const mpl::Index_t<index, First, Types...> *>(this->address());
  }

  /***
   * Typed access which respects value category of storage
   */
  template <typename T>
  T & as() & {
    return *reinterpret_cast<T *>(this->address());
  }

  template <typename T>
  const T & as() const & {
    return *reinterpret_cast<const T *>(this->address());
  }

  template <typename T>
  T && as() && {
    return std::move(*reinterpret_cast<T *>(this->address()));
  }
};

} // end namespace detail
} // end namespace safe_variant
