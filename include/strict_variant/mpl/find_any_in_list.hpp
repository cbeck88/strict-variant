//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <strict_variant/mpl/find_with.hpp>
#include <strict_variant/mpl/typelist.hpp>

namespace strict_variant {
namespace mpl {

// A version of find_any for a typelist rather than a parameter pack
template <template <class> class P, typename TL>
struct Find_Any_In_List;

template <template <class> class P, typename... Ts>
struct Find_Any_In_List<P, mpl::TypeList<Ts...>> : Find_Any<P, Ts...> {};

} // end namespace mpl
} // end namespace strict_variant
