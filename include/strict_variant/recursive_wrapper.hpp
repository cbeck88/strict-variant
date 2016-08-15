//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * For use with strict_variant::variant
 */
#include <strict_variant/mpl/std_traits.hpp>
#include <strict_variant/variant_fwd.hpp>
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

  void destroy() {
    if (m_t) { delete m_t; }
  }

public:
  ~recursive_wrapper() noexcept { this->destroy(); }

  template <typename... Args,
            typename Dummy = mpl::enable_if_t<std::is_constructible<T, Args...>::value>>
  recursive_wrapper(Args &&... args)
    : m_t(new T(std::forward<Args>(args)...)) {}

  recursive_wrapper(const recursive_wrapper & rhs)
    : m_t(new T(rhs.get())) {}

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

//[ strict_variant_pierce_recursive_wrapper
namespace detail {

/***
 * Function to pierce the recursive_wrapper template
 */

template <typename T>
inline auto
pierce_recursive_wrapper(T && t) -> T {
  return std::forward<T>(t);
}

template <typename T>
inline T &
pierce_recursive_wrapper(recursive_wrapper<T> & t) {
  return t.get();
}

template <typename T>
inline T &&
pierce_recursive_wrapper(recursive_wrapper<T> && t) {
  return std::move(t.get());
}

template <typename T>
inline const T &
pierce_recursive_wrapper(const recursive_wrapper<T> & t) {
  return t.get();
}

} // end namespace detail
//]

/***
 * Trait to remove the wrapper
 */
template <typename T>
struct unwrap_type {
  typedef T type;
};

template <typename T>
struct unwrap_type<recursive_wrapper<T>> {
  typedef T type;
};

template <typename T>
using unwrap_type_t = typename unwrap_type<T>::type;

/***
 * Trait to add the wrapper if a type is not no-throw move constructible
 */

template <typename T, typename = mpl::enable_if_t<std::is_nothrow_destructible<T>::value
                                                  && !std::is_reference<T>::value>>
struct wrap_if_throwing_move {
  using type = typename std::conditional<std::is_nothrow_move_constructible<T>::value, T,
                                         recursive_wrapper<T>>::type;
};

template <typename T>
using wrap_if_throwing_move_t = typename wrap_if_throwing_move<T>::type;

} // end namespace strict_variant

#undef STRICT_VARIANT_ASSERT
