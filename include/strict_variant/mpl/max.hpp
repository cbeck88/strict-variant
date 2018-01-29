//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

namespace strict_variant {
namespace detail {

/***
 * Self contained implementation of 'std::max' of a predicate over a list of
 * arguments
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

namespace mpl {
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
} // end namespace mpl
} // end namespace strict_variant
