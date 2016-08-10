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



void test_three() {
//[ strict_variant_lambda_visitor
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

int
main() {
  test_one();
  test_two();
  test_three();
}
