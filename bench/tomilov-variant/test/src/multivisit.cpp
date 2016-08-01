#include "test/versatile.hpp"
#include "test/variant.hpp"
#include "test/boost_variant.hpp"
#include "test/eggs_variant.hpp"
#include "test/common.hpp"
#include "test/wrappers.hpp"
#include "test/visit.hpp"

#include <cstdlib>

int
main()
{
    using namespace ::versatile;
    using namespace ::test_boost_variant;
    { // multivisit mixed visitables
        struct A {};
        struct B {};

        struct
        {
            int operator () (A, A) { return 0; }
            int operator () (A, B) { return 1; }
            int operator () (B, A) { return 2; }
            int operator () (B, B) { return 3; }
        } v;

        A a;
        B b;

        using U = versatile< A, B >;
        using V = variant< A, B >;

        CHECK(multivisit(v, U{a}, V{a}) == 0);
        CHECK(multivisit(v, U{a}, V{b}) == 1);
        CHECK(multivisit(v, U{b}, V{a}) == 2);
        CHECK(multivisit(v, U{b}, V{b}) == 3);

        CHECK(multivisit(v, V{a}, U{a}) == 0);
        CHECK(multivisit(v, V{a}, U{b}) == 1);
        CHECK(multivisit(v, V{b}, U{a}) == 2);
        CHECK(multivisit(v, V{b}, U{b}) == 3);
    }
    { // adapted variants mixed multivisitation
        struct A {};
        struct B {};

        struct
        {
            int operator () (A, A) { return 0; }
            int operator () (A, B) { return 1; }
            int operator () (B, A) { return 2; }
            int operator () (B, B) { return 3; }
        } v;

        A a;
        B b;

        using U = boost_variant_c< A, B >;
        using V = boost_variant_i< A, B >;

        CHECK(multivisit(v, U{a}, V{a}) == 0);
        CHECK(multivisit(v, U{a}, V{b}) == 1);
        CHECK(multivisit(v, U{b}, V{a}) == 2);
        CHECK(multivisit(v, U{b}, V{b}) == 3);

        CHECK(multivisit(v, V{a}, U{a}) == 0);
        CHECK(multivisit(v, V{a}, U{b}) == 1);
        CHECK(multivisit(v, V{b}, U{a}) == 2);
        CHECK(multivisit(v, V{b}, U{b}) == 3);
    }
    return EXIT_SUCCESS;
}
