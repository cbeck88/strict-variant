//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

/***
 * Enable streaming operations for variant types
 */

#include <iosfwd>
#include <strict_variant/variant.hpp>

namespace strict_variant {

// printer
struct printer_visitor {
  typedef void result_type;
  explicit printer_visitor(std::ostream & ss)
    : m_ss(ss) {}

  template <typename T>
  void operator()(const T & t) const {
    m_ss << t;
  }

private:
  std::ostream & m_ss;
};

/***
 * ostream op
 */
template <typename First, typename... Types>
std::ostream &
operator<<(std::ostream & s, const variant<First, Types...> & v) {
  printer_visitor vis{s};
  strict_variant::apply_visitor(vis, v);
  return s;
}

} // end namespace strict_variant
