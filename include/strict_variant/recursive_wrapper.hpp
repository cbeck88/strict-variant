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

namespace strict_variant {

template <typename T>
class recursive_wrapper {
  T * m_t;

  template <typename U>
  void assign(U && u) {
    if (m_t) {
      *m_t = std::forward<U>(u);
    } else {
      m_t = new T(std::forward<U>(u));
    }
  }

  void destroy() {
    if (m_t) { delete m_t; }
  }

public:
  ~recursive_wrapper() noexcept { this->destroy(); }

  template <typename Dummy = void>
  recursive_wrapper()
    : m_t(new T()) {}

  template <typename U, typename Dummy = mpl::enable_if_t<std::is_convertible<U, T>::value>>
  recursive_wrapper(U && u)
    : m_t(new T(std::forward<U>(u))) {}

  recursive_wrapper(const recursive_wrapper & rhs)
    : m_t(new T(rhs.get())) {}

  recursive_wrapper(recursive_wrapper && rhs) noexcept //
    : m_t(rhs.m_t)                                     //
  {
    rhs.m_t = nullptr;
  }

  recursive_wrapper & operator=(
    const recursive_wrapper & rhs) // noexcept checks assign(const recursive_wrapper &)
  // TODO: Fix emscripten here:
  // noexcept( std::declval<recursive_wrapper>().assign(static_cast<const
  // recursive_wrapper
  // &>(std::declval<recursive_wrapper>())))
  {
    this->assign(rhs.get());
    return *this;
  }

  recursive_wrapper & operator=(recursive_wrapper && rhs) noexcept {
    this->destroy();
    m_t = rhs.m_t;
    rhs.m_t = nullptr;
    return *this;
  }

  recursive_wrapper & operator=(const T & t) // noexcept checks assign(const T &)
  // TODO: Emscripten broke here:
  // noexcept(std::declval<recursive_wrapper>().assign(static_cast<const T
  // &>(std::declval<T>())))
  {
    this->assign(t);
    return *this;
  }

  recursive_wrapper & operator=(T && t) // noexcept checks assign(T &&)
  // TODO: Emscripten broke here:
  // noexcept(std::declval<recursive_wrapper>().assign(std::declval<T>()))
  {
    this->assign(std::move(t));
    return *this;
  }

  T & get() & { return *m_t; }
  const T & get() const & { return *m_t; }
  T && get() && { return std::move(*m_t); }

  // TODO: This is a work-around for when we generalize the variant type and
  // need to construct `int` from `const recursive_wrapper<int>`. But we should
  // probably just insert some template code into storage obj for this case...
  operator T &() & { return this->get(); }
  operator T const &() const & { return this->get(); }
  operator T &&() && { return std::move(this->get()); }
};

namespace detail {

/***
 * Function to pierce the recursive wrapper
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
