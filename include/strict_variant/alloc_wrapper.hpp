//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * For use with strict_variant::variant
 */
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

//[ strict_variant_alloc_wrapper
namespace strict_variant {

template <typename T, typename Alloc>
class alloc_wrapper {
  T * m_t;

  void destroy() {
    if (m_t) {
      Alloc a;
      m_t->~T();
      a.deallocate(m_t, 1);
    }
  }

  // To avoid doing explicit try / catch (since some projects use -fno-exceptions),
  // use a function object to do initialization, which cleans up after itself in the dtor
  // if initialization was unsuccessful.

  struct initer {
    Alloc a;
    bool success;
    T * m_t;

    initer()
      : a()
      , success(false)
      , m_t(nullptr) {
      m_t = a.allocate(1);
    }

    ~initer() {
      if (!success) { a.deallocate(m_t, 1); }
    }

    template <typename... Args>
    void go(Args &&... args) {
      new (m_t) T(std::forward<Args>(args)...);
      success = true;
    }
  };

  template <typename... Args>
  void init(Args &&... args) {
    initer i;
    i.go(std::forward<Args>(args)...);
    m_t = i.m_t;
  }

public:
  typedef T value_type;

  ~alloc_wrapper() noexcept { this->destroy(); }

  template <typename... Args>
  alloc_wrapper(Args &&... args)
    : m_t(nullptr) {
    this->init(std::forward<Args>(args)...);
  }

  alloc_wrapper(alloc_wrapper & rhs)
    : alloc_wrapper(static_cast<const alloc_wrapper &>(rhs)) {}

  alloc_wrapper(const alloc_wrapper & rhs)
    : m_t(nullptr) {
    this->init(rhs.get());
  }

  // Pointer move
  alloc_wrapper(alloc_wrapper && rhs) noexcept //
    : m_t(rhs.m_t)                             //
  {
    rhs.m_t = nullptr;
  }

  // Not assignable, we never actually need this, and it adds complexity
  // associated to lifetime of `m_t` object.
  alloc_wrapper & operator=(const alloc_wrapper &) = delete;
  alloc_wrapper & operator=(alloc_wrapper &&) = delete;

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

template <typename T, typename A>
struct is_wrapper<alloc_wrapper<T, A>> : std::true_type {};

} // end namespace detail

} // end namespace strict_variant

#undef STRICT_VARIANT_ASSERT
