#include "test/eggs_variant.hpp"
#include "test/common.hpp"
#include "test/visit.hpp"

#include <versatile/type_traits.hpp>

#include <cstdlib>

int
main()
{
    using ::versatile::identity;
    using ::test::literal_type;
    using ::test::common_type;
    using ::test::check_indexing;
    using ::test::check_destructible;
    using ::test::perferct_forwarding;
    { // eggs::variant
        using ::test_eggs_variant::eggs_variant_c;
        {
            ASSERT ((check_indexing< identity, eggs_variant_c >::run()));
        }
        {
            //CHECK ((check_destructible< identity, eggs_variant_c >::run())); // clang-3.8: fatal error: unable to execute command: Segmentation fault (core dumped)
        }
        {
            ASSERT ((perferct_forwarding< literal_type, eggs_variant_c, identity, 2, 2 >::run()));
            CHECK ((perferct_forwarding< common_type, eggs_variant_c, identity, 2, 2 >::run()));
        }
    }
    return EXIT_SUCCESS;
}
