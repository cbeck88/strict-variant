#include "test/versatile.hpp"
#include "test/common.hpp"
#include "test/wrappers.hpp"
#include "test/visit.hpp"

#include <cstdlib>

int
main()
{
    using ::versatile::identity;
    using ::test::aggregate;
    using ::test::recursive_wrapper;
    using ::test::literal_type;
    using ::test::check_indexing;
    using ::test::check_invariants;
    using ::test::check_triviality;
    using ::test::check_utility;
    using ::test::check_destructible;
    using ::test::perferct_forwarding;
    { // versatile
        using ::versatile::versatile;
        {
            ASSERT ((check_indexing< identity,          versatile >::run()));
            ASSERT ((check_indexing< aggregate,         versatile >::run()));
            //CHECK ((check_indexing< recursive_wrapper, versatile >::run()));
        }
        {
            ASSERT ((check_invariants< identity,  versatile >::run()));
            ASSERT ((check_invariants< aggregate, versatile >::run()));
        }
        {
            ASSERT ((check_triviality< identity,  versatile >::run()));
            ASSERT ((check_triviality< aggregate, versatile >::run()));
        }
        {
            ASSERT ((check_utility< identity,  versatile >::run()));
            ASSERT ((check_utility< aggregate, versatile >::run()));
        }
        {
            CHECK ((check_destructible< identity,          versatile >::run()));
            CHECK ((check_destructible< aggregate,         versatile >::run()));
            CHECK ((check_destructible< recursive_wrapper, versatile >::run()));
        }
        {
            ASSERT ((perferct_forwarding< literal_type, versatile, identity,  2, 2 >::run()));
            ASSERT ((perferct_forwarding< literal_type, versatile, aggregate, 2, 2 >::run()));
        }
    }
    return EXIT_SUCCESS;
}
