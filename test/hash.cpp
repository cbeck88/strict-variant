#include <safe_variant/variant.hpp>
#include <safe_variant/variant_hash.hpp>
#include <safe_variant/variant_stream_ops.hpp>

#include "test_harness/test_harness.hpp"

#include <type_traits>
#include <unordered_set>
#include <unordered_map>

namespace safe_variant {

UNIT_TEST(hashing) {
  using var_t = variant<std::string, int>;
  using set_t = std::unordered_set<var_t>;

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

UNIT_TEST(copying) {
  using key_t = variant<std::string, int>;
  using val_t = variant<bool, int, double>;
  using map_t = std::unordered_map<key_t, val_t>;

  map_t a;
  TEST_EQ(a.size(), 0);

  {
    map_t m;
    m["4"] = 5.0f;
    m[4] = false;
    TEST_EQ(val_t{false}, m[4]);
    TEST_EQ(val_t{5.0f}, m["4"]);
    m.erase(4);
    TEST_EQ(m.size(), 1);

    a = m;

    TEST_EQ(m.size(), 1);
    TEST_EQ(val_t{5.0f}, m["4"]);
    m["asdf"] = 17;
    TEST_EQ(m["asdf"].which(), 1);
  }

  {
    TEST_EQ(val_t{5.0f}, a["4"]);
    TEST_EQ(a.size(), 1);
  }
}

} // end namespace safe_variant

int
main() {
  std::cout << "Variant hash tests:" << std::endl;
  return test_registrar::run_tests();
}
