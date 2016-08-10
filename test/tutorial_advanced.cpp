#include <cassert>
#include <strict_variant/variant.hpp>
#include <string>
#include <vector>
#include <iostream>

using namespace strict_variant;

//[ strict_variant_tutorial_emplace_ctor

/*`
In some cases, you might want to use `emplace` to initialize a variant to a
specific type explicitly. For instance it might be in an initializer list, and
it's not an option to use the `emplace` method.

The emplace constructor is selected using tag dispatch:

```
variant::variant(emplace_tag<T>, args...)
```

for example, in an initializer list, it looks like this:
*/

//<-
struct my_struct {
  variant<int, std::string> v;

  my_struct();
};
//->
my_struct::my_struct()
  : v(emplace_tag<std::string>{}, "foo")
{}
//]

//[ strict_variant_tutorial_generalizing_ctor
/*`
A variant can be converted to a variant over strictly more types. This is called
the "generalizing" constructor. This was also supported by `boost::variant`.
*/

//<-
void test_one() {
//->
  variant<int, double> v;
  variant<int, double, std::string> v2{v};
//<-
}
//->

//`This constructor also allows reordering the types, and adding or removing `recursive_wrapper`.

//<-
void test_two() {
//->
  variant<int, double> v;
  variant<double, int> v2{v};

  variant<recursive_wrapper<int>, double> v3;
  v3 = v;
  v2 = v3;
  v = v2;
//<-
}
//->

//]

void test_three() {
//[ strict_variant_tutorial_lambda_visitor
  //` You can also use a lambda as a visitor in some cases:

  strict_variant::variant<int, float> v;
  v = 5;

  // Prints 10
  std::cout << strict_variant::apply_visitor([](double d) -> double { return d * 2; }, v) << std::endl;

  v = 7.5f;

  // Prints 22.5
  std::cout << strict_variant::apply_visitor([](double d) -> double { return d * 3; }, v) << std::endl;
//]
}

//[ strict_variant_tutorial_generic_visitor

/*`
One of the nicest things about visitors is that they can use templates -- when
you have a visitor with many types which should be handled similarly, this can
save you much typing. It also means that a single visitor can often be equally
useful for many different variants, which helps you to be very DRY.
*/

struct formatter {
  std::string operator()(const std::string & s) const { return s; }

  template <typename T, typename = typename std::enable_if<std::is_arithmetic<T>::value>::type>
  std::string operator()(T t) const { return "[" + std::to_string(t) + "]"; }

  template <typename T>
  std::string operator()(const std::vector<T> & vec) const {
    std::string result = "{ ";
    for (unsigned int i = 0; i < vec.size(); ++i) {
      if (i) { result += ", "; }
      result += (*this)(vec[i]);
    }
    result += " }";
    return result;
  }
};

//` This generic visitor is appropriate for a wide variety of variants:

//<-
void test_four() {
//->
  variant<int, double> v1;
  variant<float, std::string, std::vector<int>> v2;
  variant<long, std::vector<std::vector<std::string>>> v3;

  apply_visitor(formatter{}, v1);
  apply_visitor(formatter{}, v2);
  apply_visitor(formatter{}, v3);
//]

}


//[ strict_variant_tutorial_throwing_assignment_goal_code

// A class with a throwing move
class X {
  std::string foo;
  std::string bar;

public:
  X () noexcept;
  X (const X &) noexcept(false);

  X & operator=(const X &) noexcept(false);
};

// A variant containing X
void goal() {
  variant<int, X> v;
//=  v = 5;
//=  v = X();  // Error: No assignment if X has a throwing move!
}

//]

X::X() noexcept {};
X::X(const X&) noexcept(false) {}
X & X::operator=(const X&) noexcept(false) { return *this; }

namespace fix1 {

//[ strict_variant_tutorial_throwing_assignment_fix1
//` Use a `recursive_wrapper`.
void goal() {
  variant<int, recursive_wrapper<X>> v;
  v = 5;
  v = X();
}
//]

} // end namespace fix1

namespace fix2 {

//[ strict_variant_tutorial_throwing_assignment_fix2
//` Use `easy_variant` instead. It's the same, but it implicitly applies wrappers to value types with a throwing move.
void goal() {
  easy_variant<int, X> v;
  v = 5;
  v = X();
}
//]

} // end namespace fix2

namespace fix3 {

//[ strict_variant_tutorial_throwing_assignment_fix3
//` Use `emplace` instead of assignment. This works even if `X` is not copyable or moveable.
void goal() {
  variant<int, X> v;
  v.emplace<int>(5);
  v.emplace<X>();
}
//]

} // end namespace fix3


int
main() {
  test_one();
  test_two();
  test_three();
  test_four();
  goal();
  fix1::goal();
  fix2::goal();
  fix3::goal();
}
