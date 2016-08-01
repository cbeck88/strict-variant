#include "test/variant.hpp"
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
    using ::test::common_type;
    using ::test::check_indexing;
    using ::test::check_common;
    using ::test::check_destructible;
    using ::test::check_runtime;
    using ::test::perferct_forwarding;
    { // variant
        using ::versatile::variant;
        {
            CHECK ((check_indexing< identity,  variant >::run()));
            CHECK ((check_indexing< aggregate, variant >::run()));
            //CHECK ((check_indexing< recursive_wrapper, variant >::run()));
        }
        {
            CHECK ((check_common< identity,  variant >::run()));
            CHECK ((check_common< aggregate, variant >::run()));
            //CHECK ((check_common< recursive_wrapper, variant >::run()));
        }
        {
            CHECK ((check_destructible< identity,  variant >::run()));
            CHECK ((check_destructible< aggregate, variant >::run()));
            //CHECK ((check_destructible< recursive_wrapper, variant >::run()));
        }
        {
            CHECK ((check_runtime< variant >::run()));
        }
        {
            CHECK ((perferct_forwarding< common_type, variant, identity,          2, 2 >::run()));
            CHECK ((perferct_forwarding< common_type, variant, aggregate,         2, 2 >::run()));
            CHECK ((perferct_forwarding< common_type, variant, recursive_wrapper, 2, 2 >::run()));
        }
    }
    return EXIT_SUCCESS;
}
