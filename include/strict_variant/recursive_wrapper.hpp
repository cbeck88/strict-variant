//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * For use with strict_variant::variant
 */
#include <new>
#include <strict_variant/variant_fwd.hpp>
#include <strict_variant/wrapper.hpp>
#include <type_traits>
#include <utility>

// #define STRICT_VARIANT_DEBUG

#ifdef STRICT_VARIANT_DEBUG
#include <cassert>

#define STRICT_VARIANT_ASSERT(X, C)                                                                \
  do {                                                                                             \
    assert((X) && C);                                                                              \
  } while (0)

#else // STRICT_VARIANT_DEBUG

#define STRICT_VARIANT_ASSERT(X, C)                                                                \
  do {                                                                                             \
  } while (0)

#endif // STRICT_VARIANT_DEBUG

//[ strict_variant_recursive_wrapper
namespace strict_variant {

template <typename T>
class recursive_wrapper {
  T * m_t;

  void destroy() { delete m_t; }

  template <typename... Args>
  void init(Args &&... args) {
    m_t = new T(std::forward<Args>(args)...);
  }

public:
  typedef T value_type;

  ~recursive_wrapper() noexcept { this->destroy(); }

  template <typename... Args>
  recursive_wrapper(Args &&... args)
    : m_t(nullptr) {
    this->init(std::forward<Args>(args)...);
  }

  recursive_wrapper(recursive_wrapper & rhs)
    : recursive_wrapper(static_cast<const recursive_wrapper &>(rhs)) {}

  recursive_wrapper(const recursive_wrapper & rhs)
    : m_t(nullptr) {
    this->init(rhs.get());
  }

  // Pointer move
  recursive_wrapper(recursive_wrapper && rhs) noexcept //
    : m_t(rhs.m_t)                                     //
  {
    rhs.m_t = nullptr;
  }

  // Not assignable, we never actually need this, and it adds complexity
  // associated to lifetime of `m_t` object.
  recursive_wrapper & operator=(const recursive_wrapper &) = delete;
  recursive_wrapper & operator=(recursive_wrapper &&) = delete;

  T & get() & {
    STRICT_VARIANT_ASSERT(m_t, "Bad access!");
    return *m_t;
  }
  const T & get() const & {
    STRICT_VARIANT_ASSERT(m_t, "Bad access!");
    return *m_t;
  }
  T && get() && {
    STRICT_VARIANT_ASSERT(m_t, "Bad access!");
    return std::move(*m_t);
  }
};
//]

namespace detail {

template <typename T>
struct is_wrapper<recursive_wrapper<T>> : std::true_type {};

} // end namespace detail

} // end namespace strict_variant

#undef STRICT_VARIANT_ASSERT
