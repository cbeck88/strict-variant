#include <variant>
#include "example.hpp"

int main() {
  using var_t = std::experimental::variant<A, B>;
  
  var_t v{A()};
  v.emplace(B());
  v.emplace(A());
}
