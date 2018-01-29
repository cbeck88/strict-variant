//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>
#include <functional>
#include <strict_variant/variant.hpp>

//- hash support:
namespace std {

template <typename... Ts>
struct hash<strict_variant::variant<Ts...>> {

  using argument_type = strict_variant::variant<Ts...>;
  using result_type = std::size_t;

private:
  struct hasher {
    using result_type = std::size_t;
    template <typename Arg>
    std::size_t operator()(const Arg & arg) const {
      return std::hash<Arg>{}(arg);
    }
  }; // hasher

public:
  std::size_t operator()(const argument_type & v) const {
    return strict_variant::apply_visitor(hasher{}, v) + (31 * v.which());
  }
}; // hash<strict_variant::variant<Ts...>>

} // namespace std
