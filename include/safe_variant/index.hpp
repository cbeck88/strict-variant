//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Simple 'index' metafunction
 */

namespace safe_variant {
namespace mpl {

template <size_t s, typename... Types>
struct Index;

template <size_t s, typename First, typename... Types>
struct Index<s, First, Types...> : public Index<s - 1, Types...> {};

template <typename First, typename... Types>
struct Index<0, First, Types...> {
  typedef First type;
};

template <size_t s, typename... Types>
using Index_t = typename Index<s, Types...>::type;

} // end namespace mpl
} // end namespace safe_variant
