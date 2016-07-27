
#include <boost/spirit/include/qi.hpp>
#include <safe_variant/variant.hpp>
#include <safe_variant/variant_spirit.hpp>
#include <safe_variant/variant_stream_ops.hpp>

#include "test_harness/test_harness.hpp"

#include <string>
#include <type_traits>

namespace qi = boost::spirit::qi;

using str_it = std::string::const_iterator;
using var_one_t = safe_variant::variant<std::string, int>;


template <typename Iterator>
struct test_grammar : qi::grammar<Iterator, var_one_t()> {
  qi::rule<Iterator, std::string()> str_;
  qi::rule<Iterator, int()> int_;
  qi::rule<Iterator, var_one_t()> main_;

  test_grammar() : test_grammar::base_type(main_) {
    str_ = qi::lit("s") >> *qi::char_;
    int_ = qi::lit("i") >> qi::int_;
    main_ = str_ | int_;
  }
};

UNIT_TEST(string_int_grammar) {
  var_one_t v;

  test_grammar<str_it> gram;

  std::string t1 = "sabc";
  std::string t2 = "i120";
  std::string t3 = "si120";
  std::string t4 = "i0";
  std::string t5 = "ssabci120";
  std::string t6 = "is";
  std::string t7 = "i70sabc";

  {
    str_it it = t1.begin(), end = t1.end();
    TEST_TRUE(qi::parse(it, end, gram, v));
    const std::string * s = safe_variant::get<std::string>(&v);
    TEST_TRUE(s);
    TEST_EQ(*s, "abc");
  }

  {
    str_it it = t2.begin(), end = t2.end();
    TEST_TRUE(qi::parse(it, end, gram, v));
    const int * i = safe_variant::get<int>(&v);
    TEST_TRUE(i);
    TEST_EQ(*i, 120);
  }

  {
    str_it it = t3.begin(), end = t3.end();
    TEST_TRUE(qi::parse(it, end, gram, v));
    const std::string * s = safe_variant::get<std::string>(&v);
    TEST_TRUE(s);
    TEST_EQ(*s, "i120");
  }

  {
    str_it it = t4.begin(), end = t4.end();
    TEST_TRUE(qi::parse(it, end, gram, v));
    const int * i = safe_variant::get<int>(&v);
    TEST_TRUE(i);
    TEST_EQ(*i, 0);
  }

  {
    str_it it = t5.begin(), end = t5.end();
    TEST_TRUE(qi::parse(it, end, gram, v));
    const std::string * s = safe_variant::get<std::string>(&v);
    TEST_TRUE(s);
    TEST_EQ(*s, "sabci120");
  }

  {
    str_it it = t6.begin(), end = t6.end();
    TEST_FALSE(qi::parse(it, end, gram, v));
  }

  {
    str_it it = t7.begin(), end = t7.end();
    TEST_TRUE(qi::parse(it, end, gram, v));
    const int * i = safe_variant::get<int>(&v);
    TEST_TRUE(i);
    TEST_EQ(*i, 70);
  }
}


int
main() {

  std::cout << "Variant spirit tests:" << std::endl;
  test_registrar::run_tests();
}
