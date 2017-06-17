//  (C) Copyright 2016 - 2017 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * For use with strict_variant::variant
 */
#include <new>
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

//[ strict_variant_is_recursive_wrapper
namespace detail {

/***
 * Trait to identify wrapper types like recursive_wrapper.
 * Specialize this trait to make strict_variant recognize and automatically
 * pierce custom wrappers.
 */

template <typename T>
struct is_wrapper : std::false_type {};

template <typename T>
struct is_wrapper<recursive_wrapper<T>> : std::true_type {};

} // end namespace detail
//]

//[ strict_variant_pierce_recursive_wrapper
namespace detail {

/***
 * Function to pierce a recursive_wrapper
 */

template <typename T>
inline auto
pierce_recursive_wrapper(T && t)
  -> mpl::enable_if_t<!is_wrapper<mpl::remove_const_t<mpl::remove_reference_t<T>>>::value, T> {
  return std::forward<T>(t);
}

template <typename T>
inline auto
pierce_recursive_wrapper(T && t)
  -> mpl::enable_if_t<is_wrapper<mpl::remove_const_t<mpl::remove_reference_t<T>>>::value,
                      decltype(std::forward<T>(t).get())> {
  return std::forward<T>(t).get();
}

} // end namespace detail
  //]

/***
 * Trait to remove a wrapper from a wrapped type
 */
template <typename T, bool is_wrapped = detail::is_wrapper<T>::value>
struct unwrap_type;

template <typename T>
struct unwrap_type<T, false> {
  typedef T type;
};

template <typename T>
struct unwrap_type<T, true> {
  typedef typename T::value_type type;
};

template <typename T>
using unwrap_type_t = typename unwrap_type<T>::type;

} // end namespace strict_variant

#undef STRICT_VARIANT_ASSERT
