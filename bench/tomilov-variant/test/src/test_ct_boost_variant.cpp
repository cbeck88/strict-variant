#include "test/deep_and_hard.hpp"

#include <boost/variant.hpp>
#include <boost/variant/multivisitors.hpp>

#include <utility>

#include <cstdlib>

namespace test
{

template< std::size_t ...M, std::size_t ...N >
bool
hard(std::index_sequence< M... >, std::index_sequence< N... >) noexcept
{
    using V = ::boost::variant< T< M >... >;
    V variants_[sizeof...(N)] = {{T< (N % sizeof...(M)) >{}}...};
    visitor< sizeof...(N) > visitor_;
    auto const rhs_ = apply_visitor(visitor_, variants_[N]...);
    decltype(rhs_) lhs_ = {{{N % sizeof...(M)}...}};
    return ((lhs_[N] == rhs_[N]) && ...);
}

}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"

#if 4 < ROWS
#undef ROWS
#define ROWS 4
#warning "ROWS is greater then 4, defaulted to 4"
#endif

#if 4 < COLS
#undef COLS
#define COLS 4
#warning "COLS is greater then 4, defaulted to 4"
#endif

#pragma clang diagnostic pop

int
main()
{
    CHECK ((test::run< ROWS, COLS >()));
    return EXIT_SUCCESS;
}
