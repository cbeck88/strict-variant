//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <safe_variant/variant.hpp>
#include <safe_variant/variant_compare.hpp>
#include <safe_variant/variant_stream_ops.hpp>

#include "test_harness/test_harness.hpp"

#include <set>
#include <string>
#include <type_traits>

// Test that variant_comparator works

using var_t = safe_variant::variant<std::string, int>;
using set_t = std::set<var_t, safe_variant::variant_comparator<var_t>>;

void test_one() {
  set_t s;
  s.insert(var_t{"asdf"});
  s.insert(var_t{"jkl;"});
  s.insert(var_t{0});
  s.insert(var_t{1});

  TEST_EQ(4, s.size());
  TEST_TRUE(s.count(var_t("asdf")));
  TEST_TRUE(s.count(var_t("jkl;")));
  TEST_TRUE(s.count(var_t(0)));
  TEST_TRUE(s.count(var_t(1)));

  s.erase(var_t{0});
  TEST_EQ(3, s.size());
  TEST_FALSE(s.count(var_t(0)));
  TEST_TRUE(s.count(var_t(1)));

  TEST_FALSE(s.count(var_t(70)));
}

int
main() {

  std::cout << "Variant comparator tests:" << std::endl;
  test_harness tests{
    {"test_one", &test_one},
  };
  int num_fails = tests.run();
  std::cout << "\n";
  if (num_fails) {
    std::cout << num_fails << " tests failed!" << std::endl;
    return 1;
  } else {
    std::cout << "All tests passed!" << std::endl;
    return 0;
  }
}
