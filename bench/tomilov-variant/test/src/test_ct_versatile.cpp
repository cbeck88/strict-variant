#include "test/deep_and_hard.hpp"

#include <versatile.hpp>

#include <utility>

#include <cstdlib>

namespace test
{

template< std::size_t ...M, std::size_t ...N >
bool
hard(std::index_sequence< M... >, std::index_sequence< N... >) noexcept
{
    using V = ::versatile::versatile< T< M >... >;
    V variants_[sizeof...(N)] = {{T< (N % sizeof...(M)) >{}}...};
    visitor< sizeof...(N) > visitor_;
    auto const rhs_ = multivisit(visitor_, variants_[N]...);
    decltype(rhs_) lhs_ = {{{N % sizeof...(M)}...}};
    return ((lhs_[N] == rhs_[N]) && ...);
}

}

int
main()
{
    CHECK ((test::run< ROWS, COLS >()));
    return EXIT_SUCCESS;
}
