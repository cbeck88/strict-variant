//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Some metafunctions that support safely_constructible type_trait
 *
 * Permits to classify all fundamental types
     integral, character, wide_char, bool, floating point
 * and assign numeric ranks to them which are *portable*.
 *
 * These ranks should be the same on any standards-conforming implementation of
 * C++, if not, it is a bug in this header.
 */

#include <type_traits>

namespace strict_variant {
namespace mpl {

enum class numeric_class : char { integer, character, wide_char, boolean, floating };

template <typename T, typename ENABLE = void>
struct classify_numeric;

template <typename T>
struct classify_numeric<T, typename std::enable_if<std::is_integral<T>::value>::type> {
  static constexpr numeric_class value = numeric_class::integer;
};

#define CLASSIFY(T, C)                                                                             \
  template <>                                                                                      \
  struct classify_numeric<T, void> {                                                               \
    static constexpr numeric_class value = numeric_class::C;                                       \
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
struct rank_numeric;

#define RANK(T, V)                                                                                 \
  template <>                                                                                      \
  struct rank_numeric<T> {                                                                         \
    static constexpr int value = V;                                                                \
  }

#define URANK(T, V)                                                                                \
  RANK(T, V);                                                                                      \
  RANK(unsigned T, V)

RANK(bool, 0);

RANK(signed char, 0);
URANK(char, 1);
RANK(char16_t, 2);
RANK(char32_t, 3);

URANK(short, 1);
URANK(int, 2);
URANK(long, 3);
URANK(long long, 4);

RANK(float, 0);
RANK(double, 1);
RANK(long double, 2);

#undef RANK

/***
 * Compare two types to see if they are same class, sign and rank A >= rank B,
 * or it is a signed -> unsigned conversion of same rank
 *
 * Rationale:
 * For two integral types, signed -> unsigned is allowed if they have the same
 * rank.
 *
 * This conversion is well-defined by the standard to be a no-op for most
 * implementations.
 * (Since it means there is no truncation.)
 * [conv.integral][2]
 *
 * For things like signed char -> unsigned int, we don't allow it, since it's
 * potentially confusing that this won't be the same as
 * signed char -> unsigned char -> unsigned int.
 */
template <typename A, typename B>
struct safe_by_rank {
  static constexpr bool same_class =
    (mpl::classify_numeric<A>::value == mpl::classify_numeric<B>::value);
  static constexpr bool sa = std::is_unsigned<A>::value && !std::is_same<char, A>::value;
  static constexpr bool sb = std::is_unsigned<B>::value && !std::is_same<char, B>::value;

  static constexpr bool same_sign = (sa == sb);
  static constexpr bool sign_to_unsign = (sa && !sb);

  static constexpr int ra = mpl::rank_numeric<A>::value;
  static constexpr int rb = mpl::rank_numeric<B>::value;

  static constexpr bool value =
    same_class && (same_sign ? (ra >= rb) : (sign_to_unsign && ra == rb));

  static constexpr int priority = (5 - ra);
};

// Technically, the standard specifies that `char, signed char, unsigned char`
// have the same rank.
//
// However, we can't do that because we only want to allow conversions that are
// portable everywhere, and `char` has ambiguous sign, so we don't want to allow
// char -> signed char.
//
// As a hack, we defined rank of signed char to be -1 above,
// because that almost gets the correct behavior everywhere.
// We just need this fixup:

template <>
struct safe_by_rank<unsigned char, signed char> {
  static constexpr bool value = true;
  static constexpr int priority = 4;
};

static constexpr int priority_max = 5;

} // end namespace mpl
} // end namespace strict_variant
