//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Some metafunctions that support safely_constructible type_trait
 *
 * Permits to classify all fundamental arithmetic types
     integral, character, wide_char, bool, floating point
 *
 * and assign numeric ranks to them which are *portable*.
 *
 * These ranks should be the same on any standards-conforming implementation of
 * C++, if not, it is a bug in this header.
 */

#include <type_traits>

//[ strict_variant_arithmetic_category
namespace strict_variant {
namespace mpl {

enum class arithmetic_category : char { integer, character, wide_char, boolean, floating };

template <typename T, typename ENABLE = void>
struct classify_arithmetic;

} // end namespace mpl
} // end namespace strict_variant
//]

namespace strict_variant {
namespace mpl {

template <typename T>
struct classify_arithmetic<T, typename std::enable_if<std::is_integral<T>::value>::type> {
  static constexpr arithmetic_category value = arithmetic_category::integer;
};

#define CLASSIFY(T, C)                                                                             \
  template <>                                                                                      \
  struct classify_arithmetic<T, void> {                                                            \
    static constexpr arithmetic_category value = arithmetic_category::C;                           \
  }

CLASSIFY(char, character);
CLASSIFY(signed char, character);
CLASSIFY(unsigned char, character);
CLASSIFY(char16_t, character);
CLASSIFY(char32_t, character);
CLASSIFY(wchar_t, wide_char);
CLASSIFY(bool, boolean);
CLASSIFY(float, floating);
CLASSIFY(double, floating);
CLASSIFY(long double, floating);

#undef CLASSIFY

template <typename T>
struct arithmetic_rank;

#define RANK(T, V)                                                                                 \
  template <>                                                                                      \
  struct arithmetic_rank<T> {                                                                      \
    static constexpr int value = V;                                                                \
  }

#define URANK(T, V)                                                                                \
  RANK(T, V);                                                                                      \
  RANK(unsigned T, V)

RANK(bool, 0);

RANK(signed char, 0);
RANK(char, 0);
RANK(unsigned char, 0);
RANK(char16_t, 1);
RANK(char32_t, 2);

URANK(short, 0);
URANK(int, 1);
URANK(long, 2);
URANK(long long, 3);

RANK(float, 0);
RANK(double, 1);
RANK(long double, 2);

#undef RANK

} // end namespace mpl
} // end namespace strict_variant
