//  (C) Copyright 2016 Christopher Beck

//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <safe_variant/variant.hpp>
#include <safe_variant/variant_stream_ops.hpp>

#include "test_harness/test_harness.hpp"

#include <type_traits>

namespace safe_variant {

////////////////////////////////
// SAFELY_CONSTRUCTIBLE TESTS //
////////////////////////////////

namespace {
using namespace mpl;

static_assert(safely_constructible<bool, bool>::value, "failed a unit test");
static_assert(!safely_constructible<bool, int>::value, "failed a unit test");
static_assert(!safely_constructible<int, bool>::value, "failed a unit test");
static_assert(safely_constructible<int, int>::value, "failed a unit test");

static_assert(!safely_constructible<bool, signed char>::value, "failed a unit test");
static_assert(!safely_constructible<signed char, bool>::value, "failed a unit test");
static_assert(safely_constructible<signed char, signed char>::value, "failed a unit test");

static_assert(!safely_constructible<int, signed char>::value, "failed a unit test");
static_assert(!safely_constructible<signed char, int>::value, "failed a unit test");

static_assert(safely_constructible<char, signed char>::value, "failed a unit test");
// TODO: Can fix this somehow? It's not that important, we don't use signed
// char...
// static_assert(safely_constructible<unsigned char, signed char>::value,
// "failed a unit test");
static_assert(safely_constructible<unsigned char, char>::value, "failed a unit test");
static_assert(!safely_constructible<signed char, char>::value, "failed a unit test");
static_assert(!safely_constructible<signed char, unsigned char>::value, "failed a unit test");
static_assert(!safely_constructible<char, unsigned char>::value, "failed a unit test");

static_assert(!safely_constructible<unsigned int, unsigned long>::value, "failed a unit test");
static_assert(safely_constructible<unsigned long, unsigned int>::value, "failed a unit test");
static_assert(safely_constructible<unsigned long, unsigned long>::value, "failed a unit test");

static_assert(!safely_constructible<unsigned int, unsigned char>::value, "failed a unit test");
static_assert(!safely_constructible<unsigned char, unsigned int>::value, "failed a unit test");
static_assert(safely_constructible<unsigned char, unsigned char>::value, "failed a unit test");

static_assert(!safely_constructible<bool, unsigned char>::value, "failed a unit test");
static_assert(!safely_constructible<unsigned char, bool>::value, "failed a unit test");

static_assert(safely_constructible<float, float>::value, "failed a unit test");
static_assert(safely_constructible<double, float>::value, "failed a unit test");
static_assert(!safely_constructible<float, double>::value, "failed a unit test");
static_assert(safely_constructible<double, double>::value, "failed a unit test");

static_assert(!safely_constructible<bool, float>::value, "failed a unit test");
static_assert(!safely_constructible<float, bool>::value, "failed a unit test");

static_assert(!safely_constructible<bool, double>::value, "failed a unit test");
static_assert(!safely_constructible<double, bool>::value, "failed a unit test");

static_assert(!safely_constructible<int, float>::value, "failed a unit test");
static_assert(!safely_constructible<float, int>::value, "failed a unit test");

static_assert(!safely_constructible<signed int, unsigned int>::value, "failed a unit test");
static_assert(safely_constructible<unsigned int, signed int>::value, "failed a unit test");

static_assert(!safely_constructible<signed char, unsigned char>::value, "failed a unit test");
static_assert(!safely_constructible<unsigned char, signed char>::value, "failed a unit test");

static_assert(!safely_constructible<signed char, unsigned int>::value, "failed a unit test");
static_assert(!safely_constructible<unsigned int, signed char>::value, "failed a unit test");

static_assert(!safely_constructible<double, int>::value, "failed a unit test");
static_assert(!safely_constructible<int, double>::value, "failed a unit test");

static_assert(!safely_constructible<double, const int &>::value, "failed a unit test");
static_assert(!safely_constructible<int, const double &>::value, "failed a unit test");

// Pointer to int checks and such

static_assert(!safely_constructible<double, const int *>::value, "failed a unit test");
static_assert(!safely_constructible<int, const double *>::value, "failed a unit test");

static_assert(!safely_constructible<int, const char *>::value, "failed a unit test");
static_assert(!safely_constructible<const char *, int>::value, "failed a unit test");
static_assert(safely_constructible<const char *, const char *>::value, "failed a unit test");

static_assert(!safely_constructible<int &, const char *>::value, "failed a unit test");
static_assert(!safely_constructible<const char * const, int &>::value, "failed a unit test");

static_assert(!safely_constructible<int *, const char *>::value, "failed a unit test");
static_assert(!safely_constructible<const char *, int *>::value, "failed a unit test");
static_assert(safely_constructible<int *, int *>::value, "failed a unit test");

static_assert(!safely_constructible<bool, decltype("asdf")>::value, "failed a unit test");
static_assert(!safely_constructible<int, decltype("asdf")>::value, "failed a unit test");
static_assert(!safely_constructible<char, decltype("asdf")>::value, "failed a unit test");
static_assert(!safely_constructible<float, decltype("asdf")>::value, "failed a unit test");
static_assert(safely_constructible<const char *, decltype("asdf")>::value, "failed a unit test");
static_assert(!safely_constructible<const bool *, decltype("asdf")>::value, "failed a unit test");
static_assert(!safely_constructible<const int *, decltype("asdf")>::value, "failed a unit test");
static_assert(!safely_constructible<const float *, decltype("asdf")>::value, "failed a unit test");

} // end namespace mpl

/////////////////////////
// TYPELIST OPERATIONS //
/////////////////////////

struct _dummy_t_1 {};
struct _dummy_t_2 {};

static_assert(std::is_same<_dummy_t_1, Car<TypeList<_dummy_t_1, _dummy_t_2>>>::value,
              "unit test failed");


static_assert(std::is_same<TypeList<_dummy_t_2>, Cdr<TypeList<_dummy_t_1, _dummy_t_2>>>::value,
              "unit test failed");



/////////////////////////
// VARIANT TYPE TRAITS //
/////////////////////////


// Check the proper_subvariant trait, which is used to enable "generalizing"
// ctor
// without interfering with special member functions.
static_assert(detail::proper_subvariant<variant<int, double>,
                                              variant<int, double, std::string>>::value,
              "failed a unit test");

static_assert(detail::proper_subvariant<variant<int, std::string, double>,
                                              variant<int, double, std::string>>::value,
              "failed a unit test");

static_assert(!detail::proper_subvariant<variant<int, std::string, double>,
                                               variant<int, double>>::value,
              "failed a unit test");

static_assert(
  !detail::proper_subvariant<variant<int, double>, variant<int, double>>::value,
  "failed a unit test");

// Check nothrow status
static_assert(std::is_nothrow_destructible<variant<int, double>>::value,
              "failed a unit test");
static_assert(std::is_nothrow_move_constructible<variant<int, double>>::value,
              "failed a unit test");

// Check core traits that enable construction from other types
static_assert(detail::allow_variant_construction<std::string, const char *>::value,
              "failed a unit test");
static_assert(detail::allow_variant_construction<std::string, const std::string &>::value,
              "failed a unit test");
static_assert(detail::allow_variant_construction<std::string, std::string &&>::value,
              "failed a unit test");
static_assert(!detail::allow_variant_construction<int, const char *>::value, "failed a unit test");
static_assert(detail::allow_variant_construction<int, int>::value, "failed a unit test");
static_assert(!detail::allow_variant_construction<int, double>::value, "failed a unit test");
static_assert(!detail::allow_variant_construction<double, int>::value, "failed a unit test");
static_assert(detail::allow_variant_construction<double, double>::value, "failed a unit test");

static_assert(detail::allow_variant_construction<int, const int &>::value, "failed a unit test");
static_assert(!detail::allow_variant_construction<int, const double &>::value,
              "failed a unit test");
static_assert(!detail::allow_variant_construction<double, const int &>::value,
              "failed a unit test");
static_assert(detail::allow_variant_construction<double, const double &>::value,
              "failed a unit test");

static_assert(detail::allow_variant_construction<int, int &&>::value, "failed a unit test");
static_assert(!detail::allow_variant_construction<int, double &&>::value, "failed a unit test");
static_assert(!detail::allow_variant_construction<double, int &&>::value, "failed a unit test");
static_assert(detail::allow_variant_construction<double, double &&>::value, "failed a unit test");

static_assert(detail::allow_variant_construction<float, float &&>::value, "failed a unit test");
static_assert(!detail::allow_variant_construction<float, double &&>::value, "failed a unit test");
static_assert(detail::allow_variant_construction<double, float &&>::value, "failed a unit test");

// Testing typlist


#define UNIT_TEST(NAME) void NAME ()

// Check that variant is resolving "ambiguous" constructions as expected
UNIT_TEST(variant_0) {
  const char * test_string = "asdf";
  {
    typedef variant<std::string, const char *> Var_t;
    Var_t a{test_string};
    TEST_EQ(a.which(), 0);
    TEST_TRUE(a.get<std::string>());
    TEST_FALSE(a.get<const char *>());
  }
  {
    typedef variant<const char *, std::string> Var_t;
    Var_t a{test_string};
    TEST_EQ(a.which(), 0);
    TEST_FALSE(a.get<std::string>());
    TEST_TRUE(a.get<const char *>());
  }
}

// Test that variants using integral types are working as expected
UNIT_TEST(variant_1) {
  {
    typedef variant<double, float, int> Var_t;
    Var_t a{5};
    TEST_EQ(a.which(), 2);
    TEST_FALSE(a.get<double>());
    TEST_TRUE(a.get<int>());
    Var_t b{10.0f};
    TEST_EQ(b.which(), 0);
    TEST_TRUE(b.get<double>());
    TEST_FALSE(b.get<int>());
    TEST_FALSE(b.get<float>());
    Var_t c{10.0};
    TEST_EQ(c.which(), 0);
    TEST_TRUE(c.get<double>());
  }

  {
    typedef variant<int, float, double> Var_t2;
    Var_t2 a{5};
    TEST_EQ(a.which(), 0);
    TEST_TRUE(a.get<int>());
    Var_t2 b{10.0f};
    TEST_EQ(b.which(), 1);
    TEST_TRUE(b.get<float>());
    Var_t2 c{10.0};
    TEST_EQ(c.which(), 2);
    TEST_TRUE(c.get<double>());
  }

  {
    typedef variant<bool, int, float> Var_t3;
    Var_t3 a{true};
    TEST_EQ(a.which(), 0);
    TEST_TRUE(a.get<bool>());
    Var_t3 b{1};
    TEST_EQ(b.which(), 1);
    TEST_FALSE(b.get<bool>());
    TEST_TRUE(b.get<int>());
    Var_t3 c{10.0f};
    TEST_EQ(c.which(), 2);
    TEST_TRUE(c.get<float>());
  }

  {
    typedef variant<int, bool, float> Var_t4;
    Var_t4 a{true};
    TEST_EQ(a.which(), 1);
    TEST_FALSE(a.get<int>());
    TEST_TRUE(a.get<bool>());
    Var_t4 b{1};
    TEST_EQ(b.which(), 0);
    TEST_TRUE(b.get<int>());
    Var_t4 c{10.0f};
    TEST_EQ(c.which(), 2);
    TEST_TRUE(c.get<float>());
  }
}

// Test assignment

UNIT_TEST(variant_3) {
  typedef variant<int, float, double, std::string> Var_t;

  Var_t a{5};
  TEST_TRUE(a.get<int>());
  TEST_FALSE(a.get<double>());

  a = "1234";
  TEST_FALSE(a.get<int>());
  TEST_FALSE(a.get<double>());
  TEST_TRUE(a.get<std::string>());

  a = 1234.0;
  TEST_TRUE(a.get<double>());

  a = 1234;
  TEST_TRUE(a.get<int>());

  a = 1234.0f;
  TEST_TRUE(a.get<float>());

  a = 1234.0;
  TEST_TRUE(a.get<double>());
}

// Test Emplace function

UNIT_TEST(variant_4) {
  typedef variant<int, float, double, std::string> Var_t;

  Var_t a{5};
  TEST_TRUE(a.get<int>());

  a.emplace<std::string>("1234");
  TEST_TRUE(a.get<std::string>());

  a.emplace<double>(1234.0);
  TEST_TRUE(a.get<double>());

  a.emplace<int>(1234);
  TEST_TRUE(a.get<int>());

  a.emplace<float>(1234.0f);
  TEST_TRUE(a.get<float>());

  a.emplace<double>(1234.0);
  TEST_TRUE(a.get<double>());

  a.emplace<float>(1234);
  TEST_TRUE(a.get<float>());

  a.emplace<double>(1234);
  TEST_TRUE(a.get<double>());
  TEST_EQ(a, Var_t(1234.0));

  a.emplace<int>(1234.0);
  TEST_TRUE(a.get<int>());
  TEST_EQ(a, Var_t(1234));
}

// Test equality

UNIT_TEST(variant_5) {
  typedef variant<int, float, double, std::string> Var_t;

  Var_t a{5};

  TEST_EQ(a, 5);
  a.emplace<float>(5);
  TEST_EQ(a, 5.0f);
  TEST_NE(a, 5);
  TEST_NE(a, "asdf");
  a = "asdf";
  TEST_EQ(a, "asdf");

  typedef variant<bool, int, float, std::string> Var_t2;
  Var_t2 b{true};
  TEST_NE(b, Var_t2{1});
  TEST_NE(b, Var_t2{1.0f});
  TEST_NE(b, Var_t2{"true"});
  TEST_EQ(b, Var_t2{true});
  b = false;
  TEST_EQ(b, Var_t2{false});
  TEST_NE(b, Var_t2{true});
  TEST_NE(b, Var_t2{0});
  TEST_NE(b, Var_t2{0.0f});
  TEST_NE(b, Var_t2{"false"});
  b = "false";
  TEST_EQ(b, Var_t2{"false"});
  TEST_NE(b, Var_t2{false});
  TEST_NE(b, Var_t2{0});
  TEST_NE(b, Var_t2{0.0f});
  TEST_NE(b, Var_t2{true});
  TEST_NE(b, Var_t2{1});
  TEST_NE(b, Var_t2{1.0f});
  TEST_NE(b, Var_t2{"true"});
}

// Test recrusive wrapper, promotion, etc.
struct dummy;
struct crummy;

typedef variant<recursive_wrapper<dummy>, recursive_wrapper<crummy>> variant_type_2;

struct dummy {
  const char * str;
};

struct crummy {
  int val;
};

typedef variant<int, dummy> variant_type_1;

UNIT_TEST(variant_6) {
  variant_type_1 v1(5);

  TEST_EQ(v1.which(), 0);

  {
    int * temp = get<int>(&v1);
    TEST_TRUE(temp);
    TEST_EQ(*temp, 5);
  }

  const char * foo_str = "asdf";
  dummy foo{foo_str};

  v1 = foo;

  TEST_EQ(v1.which(), 1);

  {
    dummy * temp = get<dummy>(&v1);
    TEST_TRUE(temp);
    TEST_EQ(temp->str, foo_str);
  }

  v1 = 10;

  TEST_EQ(v1.which(), 0);

  {
    int * temp = get<int>(&v1);
    TEST_TRUE(temp);
    TEST_EQ(*temp, 10);
  }

  variant_type_2 v2{crummy{5}};
  TEST_EQ(v2.which(), 1);
  {
    crummy * temp = get<crummy>(&v2);
    TEST_TRUE(temp);
    TEST_EQ(temp->val, 5);
  }
}

typedef variant<dummy, crummy> variant_type_3;
typedef variant<dummy, crummy, double> variant_type_4;
typedef variant<int, crummy, dummy> variant_type_5;

// Test that the variant "promotion" constructor works
UNIT_TEST(variant_7) {
  crummy c{7};

  // TODO: Enable this?
  //  variant_type_3 v3{c};
  //  variant_type_4 v4{v3};
  //  variant_type_5 v5{v3};

  {
    variant_type_4 v4{variant_type_3{c}};
    variant_type_5 v5{variant_type_3{c}};

    crummy * c4 = get<crummy>(&v4);
    crummy * c5 = get<crummy>(&v5);

    TEST_TRUE(c4);
    TEST_TRUE(c5);
    TEST_EQ(c4->val, c5->val);
  }
}

} // end namespace safe_variant

int
main() {

  std::cout << "Variant tests:" << std::endl;
  test_harness tests{
    {"ambiguious string", &safe_variant::variant_0},
    {"ambiguious integer", &safe_variant::variant_1},
    {"assignment", &safe_variant::variant_3},
    {"emplace", &safe_variant::variant_4},
    {"equality", &safe_variant::variant_5},
    {"recursive_wrapper", &safe_variant::variant_6},
    {"promotion", &safe_variant::variant_7},
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
