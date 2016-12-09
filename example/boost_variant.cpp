#include <boost/variant.hpp>
#include "example.hpp"

int main() {
  using var_t = boost::variant<A, B>;
  
  var_t v{A()};
  std::cout << "1" << std::endl;
  v = B();
  std::cout << "2" << std::endl;
  v = A();
  std::cout << "3" << std::endl;
}
