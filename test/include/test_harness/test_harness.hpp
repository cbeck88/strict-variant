//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include "test_harness/test.hpp"

#include <exception>
#include <initializer_list>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/***
 * Test harness
 */

// Test exception type. Test assertion failures throw this.

struct test_exception : std::exception {
  std::string message_;

  explicit test_exception(const std::string & m)
    : message_(m) {}

  virtual const char * what() const throw() override {
    return message_.c_str();
  }
};

/***
 * Macros which create assertions
 */

#define TEST(C, X)                                                             \
  do {                                                                         \
    if (!(C)) {                                                                \
      std::ostringstream ss__;                                                 \
      ss__ << "Condition " #C " failed (line " << __LINE__ << ") : " << X;     \
      throw test_exception{ss__.str()};                                        \
    }                                                                          \
  } while (0)

#define TEST_EQ(A, B)                                                          \
  do {                                                                         \
    if (!((A) == (B))) {                                                       \
      std::ostringstream ss__;                                                 \
      ss__ << "Condition " #A " == " #B " failed: (line " << __LINE__          \
           << ")\n        (LHS) = (" << (A) << ") , (RHS) = (" << (B) << ")";  \
      throw test_exception{ss__.str()};                                        \
    }                                                                          \
  } while (0)

#define TEST_NE(A, B)                                                          \
  do {                                                                         \
    if (((A) == (B))) {                                                        \
      std::ostringstream ss__;                                                 \
      ss__ << "Condition " #A " != " #B " failed: (line " << __LINE__          \
           << ")\n        (LHS) = (" << (A) << ") , (RHS) = (" << (B) << ")";  \
      throw test_exception{ss__.str()};                                        \
    }                                                                          \
  } while (0)


#define TEST_TRUE(C) TEST(C, "Expected true: " #C )
#define TEST_FALSE(C) TEST(!C, "Expected false: " #C )

/***
 * Test harness object
 */

struct test_harness {
  std::vector<test_record> tests_;

  explicit test_harness(std::initializer_list<test_record> list)
    : tests_(list) {}

  int run() const {
    int num_fails = 0;
    for (const auto & t : tests_) {
      bool okay = false;
      try {
        t.run();
        okay = true;
      } catch (test_exception & te) {
        t.report_fail();
        std::cout << "      A test condition was not met.\n      " << te.what()
                  << std::endl;
      } catch (std::exception & e) {
        std::cout << "      A standard exception was thrown.\n      "
                  << e.what() << std::endl;
      } catch (...) {
        std::cout << "      An unknown (exception?) was thrown.\n      !!!"
                  << std::endl;
      }
      if (okay) {
        t.report_okay();
      } else {
        ++num_fails;
      }
    }
    return num_fails;
  }
};
