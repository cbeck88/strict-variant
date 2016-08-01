#pragma once

#include "test/prologue.hpp"

#include <versatile/utility.hpp>
#include <versatile/recursive_wrapper.hpp>

#include <typeinfo>

namespace test
{

using ::versatile::is_active;

template< template< typename ... > class wrapper,
          template< typename ... > class visitable >
class check_indexing
{

    template< typename ...types >
    using V = visitable< typename wrapper< types >::type... >;

    template< typename >
    struct check_indexing_;

    template< std::size_t ...i >
    struct check_indexing_< std::index_sequence< i... > >
    {

        template< std::size_t = 0 >
        struct S
        {

        };

        using U = V< S< i >... >;

        SA(((U::template index_at_t< S< i > >::value == (sizeof...(i) - i)) && ...));
        SA(((U::template index_of_constructible_t< S< i > >::value == (sizeof...(i) - i)) && ...));

        template< std::size_t = 0 >
        struct N
        {

            N() = delete;

        };

        template< std::size_t j >
        using W = V< std::conditional_t< (i < j), N< i >, S< i > >... >;

        SA(((W< i >::template index_of_constructible_t<>::value == (sizeof...(i) - i)) && ...));

        CONSTEXPRF
        static
        bool
        run() noexcept
        {
            CHECK ((is_active< S< i > >(U{S< i >{}}) && ...));
            CHECK (((U{S< i >{}}.which() == (sizeof...(i) - i)) && ...));
            CHECK ((is_active< S< i > >(W< i >{}) && ...));
            CHECK (((W< i >{}.which() == (sizeof...(i) - i)) && ...));
            return true;
        }

    };

public :

    CONSTEXPRF
    static
    bool
    run() noexcept
    {
        CHECK (check_indexing_< std::make_index_sequence< 1 > >::run());
        CHECK (check_indexing_< std::make_index_sequence< 2 > >::run());
        CHECK (check_indexing_< std::make_index_sequence< 5 > >::run());
        return true;
    }

};

template< template< typename ... > class wrapper,
          template< typename ... > class visitable >
class check_destructible
{

    template< typename ...types >
    using V = visitable< typename wrapper< types >::type... >;

    enum class state
    {
        never_used = 0,
        default_constructed,
        copy_constructed,
        vcopy_constructed,
        move_constructed,
        cmove_constructed,
        copy_assigned,
        vcopy_assigned,
        move_assigned,
        cmove_assigned,
        moved_from,
        destructed,
    };

    SA(state::never_used == state{});

    static
    bool
    destructible() noexcept
    {
        using ::versatile::in_place;
        struct A
        {
            state s_;
            A() : s_{state::default_constructed} { ; }
            A(A const &) { s_ = state::copy_constructed; }
            A(A &) { s_ = state::vcopy_constructed; }
            A(A && a) { s_ = state::move_constructed; a.s_ = state::moved_from; }
            A & operator = (A const &) { s_ = state::copy_assigned; return *this; }
            A & operator = (A &) { s_ = state::vcopy_assigned; return *this; }
            A & operator = (A && a) { s_ = state::move_assigned; a.s_ = state::moved_from; return *this; }
            std::type_info const * * d_;
            void set(std::type_info const * & _d) { CHECK(!_d); d_ = &_d; }
            ~A() { if (!!d_) *d_ = &typeid(A); }
        };
        struct B
        {
            state s_;
            B() : s_{state::default_constructed} { ; }
            B(B const &) { s_ = state::copy_constructed; }
            B(B &) { s_ = state::vcopy_constructed; }
            B(B && a) { s_ = state::move_constructed; a.s_ = state::moved_from; }
            B & operator = (B const &) { s_ = state::copy_assigned; return *this; }
            B & operator = (B &) { s_ = state::vcopy_assigned; return *this; }
            B & operator = (B && a) { s_ = state::move_assigned; a.s_ = state::moved_from; return *this; }
            std::type_info const * * d_;
            void set(std::type_info const * & _d) { CHECK(!_d); d_ = &_d; }
            ~B() { if (!!d_) *d_ = &typeid(B); }
        };
        {
            using U = V< A, B >;
            {
                std::type_info const * d = nullptr;
                {
                    U u{};
                    CHECK (is_active< A >(u));
                    CHECK (static_cast< A & >(u).s_ == state::default_constructed);
                    static_cast< A & >(u).set(d);
                    CHECK (!d);
                }
                CHECK (!!d);
                CHECK (*d == typeid(A));
            }
            {
                std::type_info const * d = nullptr;
                {
                    U u{in_place< B >};
                    CHECK (is_active< B >(u));
                    CHECK (static_cast< B & >(u).s_ == state::default_constructed);
                    static_cast< B & >(u).set(d);
                    CHECK (!d);
                }
                CHECK (!!d);
                CHECK (*d == typeid(B));
            }
            {
                std::type_info const * d = nullptr;
                {
                    U u{in_place< A >};
                    CHECK (is_active< A >(u));
                    CHECK (static_cast< A & >(u).s_ == state::default_constructed);
                    static_cast< A & >(u).set(d);
                    CHECK (!d);
                }
                CHECK (!!d);
                CHECK (*d == typeid(A));
            }
        }
        return true;
    }

public :

    static
    bool
    run() noexcept
    {
        CHECK (destructible());
        return true;
    }

};

template< template< typename ... > class visitable >
class check_runtime
{

    template< typename incomplete_type >
    using W = ::versatile::recursive_wrapper< incomplete_type >;

    static
    bool
    recursive() noexcept
    {
        { // composition
            struct R;
            struct A {};
            using U = visitable< A, W< R > >;
            struct R { U v; };
            U v{};
            CHECK (is_active< A >(v));
            v = R{};
            CHECK (is_active< R >(v));
        }
        { // composition
            struct R;
            struct A {};
            using U = visitable< W< R >, A >;
            struct R { U v; };
            U v{A{}};
            CHECK (is_active< A >(v));
            v = R{{A{}}};
            CHECK (is_active< R >(v));
        }
        { // inheritance
            struct R;
            struct A {};
            using U = visitable< A, W< R > >;
            struct R : U { using U::U; using U::operator =; };
            U v{};
            CHECK (is_active< A >(v));
            v = R{};
            CHECK (is_active< R >(v));
        }
        { // inheritance
            struct R;
            struct A {};
            using U = visitable< W< R >, A >;
            struct R : U { using U::U; using U::operator =; };
            U v{A{}};
            CHECK (is_active< A >(v));
            v = R{A{}};
            CHECK (is_active< R >(v));
        }
        return true;
    }

public :

    static
    bool
    run() noexcept
    {
        CHECK (recursive());
        return true;
    }

};

} // namespace test
