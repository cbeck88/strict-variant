//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * safe_variant::static_visitor is exactly the same as boost::static_visitor,
 * for use with safe_variant::variant
 */

namespace safe_variant {

template <typename T = void>
class static_visitor {
public:
  typedef T result_type;
};
} // end namespace safe_variant
