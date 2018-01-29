//  (C) Copyright 2016 - 2018 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <strict_variant/alloc_variant.hpp>
#include <strict_variant/variant.hpp>

#include "test_harness/test_harness.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <type_traits>

// Test that variant with standard allocator works

using namespace strict_variant;

// Two types with throwing moves

struct A {
  A() = default;
  A(A &&) { /* std::cout << "A moved" << std::endl; */
  }
  A(const A &) { /* std::cout << "A copied" << std::endl; */
  }
  A & operator=(const A &) {
    // std::cout << "A copy assigned" << std::endl;
    return *this;
  }
};

struct B {
  B() = default;
  B(B &&) { /* std::cout << "B moved" << std::endl; */
  }
  B(const B &) { /* std::cout << "B copied" << std::endl; */
  }
  B & operator=(const B &) {
    // std::cout << "B copy assigned" << std::endl;
    return *this;
  }
};

namespace test_one {

using var1_t = alloc_variant<std::allocator>::type<std::string, int>;
using var2_t = alloc_variant<std::allocator>::type<A, B>;

} // end namespace test_one

UNIT_TEST(std_allocator) {
  test_one::var1_t a = 5;
  TEST_EQ(a.which(), 1);
  a = "bar";
  TEST_EQ(a.which(), 0);

  test_one::var2_t b = B{};
  TEST_EQ(b.which(), 1);
  b = A{};
  TEST_EQ(b.which(), 0);
  b.emplace<A>();
  TEST_EQ(b.which(), 0);
  b = A{};
  TEST_EQ(b.which(), 0);
  b.emplace<B>();
  TEST_EQ(b.which(), 1);
}

int
main() {

  std::cout << "Variant allocator tests:" << std::endl;
  return test_registrar::run_tests();
}
