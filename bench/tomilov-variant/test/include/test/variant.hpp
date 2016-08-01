#pragma once

#include "test/prologue.hpp"

#include <versatile/utility.hpp>
#include <versatile/variant.hpp>

#include <type_traits>
#include <utility>

namespace test
{

using ::versatile::is_active;

template< template< typename ... > class wrapper,
          template< typename ... > class variant >
class check_common
{

    template< typename ...types >
    using V = variant< typename wrapper< types >::type... >;

    static
    bool
    default_constructible() noexcept
    {
        struct S
        {
            S() = default;
            S(S const &) { ; }
            S(S &) { ; }
            S(S const &&) { ; }
            S(S &&) { ; }
            S & operator = (S const &) { return *this; }
            S & operator = (S &) { return *this; }
            S & operator = (S const &&) { return *this; }
            S & operator = (S &&) { return *this; }
            //~S() DESTRUCTOR
        };
        SA(std::is_default_constructible_v< S >);
        struct N {};
        SA(std::is_default_constructible_v< N >);
        {
            using U = V< S, N >;
            SA(std::is_default_constructible_v< U >);
            {
                U const v{};
                CHECK (is_active< S >(v));
            }
            {
                U v{};
                CHECK (is_active< S >(v));
            }
        }
        {
            using U = V< N, S >;
            SA(std::is_default_constructible_v< U >);
            {
                U const v{};
                CHECK (is_active< N >(v));
            }
            {
                U v{};
                CHECK (is_active< N >(v));
            }
        }
        return true;
    }

    static
    bool
    copy_move_constructible() noexcept
    {
        struct S
        {
            S() { ; }
            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;
            S & operator = (S const &) { return *this; }
            S & operator = (S &) { return *this; }
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) { return *this; }
            ~S() = default;
        };
        SA(std::is_copy_constructible_v< S >);
        SA(is_vcopy_constructible_v< S >    );
        SA(std::is_move_constructible_v< S >);
        SA(is_cmove_constructible_v< S >    );
        struct N {};
        SA(std::is_trivial_v< N >);
        {
            using U = variant< S, N >;
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            {
                U const v{N{}};
                CHECK (is_active< N >(v));
                U const w{v};
                CHECK (is_active< N >(w));
            }
            {
                U const v{S{}};
                CHECK (is_active< S >(v));
                U const w{v};
                CHECK (is_active< S >(w));
            }
            {
                U v{N{}};
                CHECK (is_active< N >(v));
                U const w{v};
                CHECK (is_active< N >(w));
            }
            {
                U v{S{}};
                CHECK (is_active< S >(v));
                U const w{v};
                CHECK (is_active< S >(w));
            }
            {
                U const v{N{}};
                CHECK (is_active< N >(v));
                U const w{std::move(v)};
                CHECK (is_active< N >(w));
            }
            {
                U const v{S{}};
                CHECK (is_active< S >(v));
                U const w{std::move(v)};
                CHECK (is_active< S >(w));
            }
            {
                U v{N{}};
                CHECK (is_active< N >(v));
                U const w{std::move(v)};
                CHECK (is_active< N >(w));
            }
            {
                U v{S{}};
                CHECK (is_active< S >(v));
                U const w{std::move(v)};
                CHECK (is_active< S >(w));
            }
        }
        {
            using U = variant< N, S >;
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            {
                U const v{N{}};
                CHECK (is_active< N >(v));
                U const w{v};
                CHECK (is_active< N >(w));
            }
            {
                U const v{S{}};
                CHECK (is_active< S >(v));
                U const w{v};
                CHECK (is_active< S >(w));
            }
            {
                U v{N{}};
                CHECK (is_active< N >(v));
                U const w{v};
                CHECK (is_active< N >(w));
            }
            {
                U v{S{}};
                CHECK (is_active< S >(v));
                U const w{v};
                CHECK (is_active< S >(w));
            }
            {
                U const v{N{}};
                CHECK (is_active< N >(v));
                U const w{std::move(v)};
                CHECK (is_active< N >(w));
            }
            {
                U const v{S{}};
                CHECK (is_active< S >(v));
                U const w{std::move(v)};
                CHECK (is_active< S >(w));
            }
            {
                U v{N{}};
                CHECK (is_active< N >(v));
                U const w{std::move(v)};
                CHECK (is_active< N >(w));
            }
            {
                U v{S{}};
                CHECK (is_active< S >(v));
                U const w{std::move(v)};
                CHECK (is_active< S >(w));
            }
        }
        return true;
    }

    static
    bool
    copy_move_assignable() noexcept
    {
        struct S
        {
            S() { ; }
            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;
            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;
            //~S() DESTRUCTOR // clang bug: if destructor is user-declared and defaulted, then defaulted assignment operators become non-trivial and marked as non-constexpr
        };
        SA(std::is_copy_assignable_v< S >);
        SA(is_vcopy_assignable_v< S >    );
        SA(std::is_move_assignable_v< S >);
        SA(is_cmove_assignable_v< S >    );
        struct N {};
        SA(std::is_trivial_v< N >);
        {
            using U = variant< S, N >;
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                U const v{N{}};
                CHECK (is_active< N >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = v;
                CHECK (is_active< N >(w));
            }
            {
                U const v{S{}};
                CHECK (is_active< S >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = v;
                CHECK (is_active< S >(w));
            }
            {
                U v{N{}};
                CHECK (is_active< N >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = v;
                CHECK (is_active< N >(w));
            }
            {
                U v{S{}};
                CHECK (is_active< S >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = v;
                CHECK (is_active< S >(w));
            }
            {
                U const v{N{}};
                CHECK (is_active< N >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = std::move(v);
                CHECK (is_active< N >(w));
            }
            {
                U const v{S{}};
                CHECK (is_active< S >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = std::move(v);
                CHECK (is_active< S >(w));
            }
            {
                U v{N{}};
                CHECK (is_active< N >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = std::move(v);
                CHECK (is_active< N >(w));
            }
            {
                U v{S{}};
                CHECK (is_active< S >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = std::move(v);
                CHECK (is_active< S >(w));
            }
        }
        {
            using U = variant< N, S >;
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                U const v{N{}};
                CHECK (is_active< N >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = v;
                CHECK (is_active< N >(w));
            }
            {
                U const v{S{}};
                CHECK (is_active< S >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = v;
                CHECK (is_active< S >(w));
            }
            {
                U v{N{}};
                CHECK (is_active< N >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = v;
                CHECK (is_active< N >(w));
            }
            {
                U v{S{}};
                CHECK (is_active< S >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = v;
                CHECK (is_active< S >(w));
            }
            {
                U const v{N{}};
                CHECK (is_active< N >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = std::move(v);
                CHECK (is_active< N >(w));
            }
            {
                U const v{S{}};
                CHECK (is_active< S >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = std::move(v);
                CHECK (is_active< S >(w));
            }
            {
                U v{N{}};
                CHECK (is_active< N >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = std::move(v);
                CHECK (is_active< N >(w));
            }
            {
                U v{S{}};
                CHECK (is_active< S >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = std::move(v);
                CHECK (is_active< S >(w));
            }
        }
        return true;
    }

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
    };

    SA(state::never_used == state{});

    struct S
    {
        state state_ = state::never_used;
        S() : state_{state::default_constructed} { ; }
        S(S const &) : state_{state::copy_constructed} { ; }
        S(S &) : state_{state::vcopy_constructed} { ; }
        //S(S const &&) { state_ = state::cmove_constructed; }
        S(S && s) : state_{state::move_constructed} { s.state_ = state::moved_from; }
        S & operator = (S const &) { state_ = state::copy_assigned; return *this; }
        S & operator = (S &) { state_ = state::vcopy_assigned; return *this; }
        //S & operator = (S const &&) { state_ = state::cmove_assigned; return *this; }
        S & operator = (S && s) { state_ = state::move_assigned; s.state_ = state::moved_from; return *this; }
        ~S() DESTRUCTOR
    };

    static
    bool
    convertible() noexcept
    {
        SA(std::is_copy_constructible_v< S >);
        SA(is_vcopy_constructible_v< S >    );
        SA(std::is_move_constructible_v< S >);
        SA(is_cmove_constructible_v< S >    );
        SA(std::is_copy_assignable_v< S >   );
        SA(is_vcopy_assignable_v< S >       );
        SA(std::is_move_assignable_v< S >   );
        SA(is_cmove_assignable_v< S >       );
        {
            using U = variant< S >;
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            SA(is_explicitly_convertible_v< U, S >);
            SA(is_explicitly_convertible_v< U const, S >);
            SA(is_explicitly_convertible_v< U &, S >);
            SA(is_explicitly_convertible_v< U const &, S >);
            {
                struct T {};
                SA(!is_explicitly_convertible_v< U, T >); // SFINAE-disabled conversions
                SA(!is_explicitly_convertible_v< U const, T >); // SFINAE-disabled conversions
                SA(!is_explicitly_convertible_v< U &, T >); // SFINAE-disabled conversions
                SA(!is_explicitly_convertible_v< U const &, T >); // SFINAE-disabled conversions
            }
            {
                U const v{};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S const & >(v).state_ == state::default_constructed);
                S const s{v};
                CHECK (static_cast< S const & >(v).state_ == state::default_constructed);
                CHECK (s.state_ == state::copy_constructed);
            }
            {
                U v{};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::default_constructed);
                S const s{v};
                CHECK (static_cast< S & >(v).state_ == state::default_constructed);
                CHECK (s.state_ == state::vcopy_constructed);
            }
            {
                U const v{};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S const & >(v).state_ == state::default_constructed);
                S const l{std::move(v)};
                CHECK (static_cast< S const & >(v).state_ == state::default_constructed);
                CHECK (l.state_ == state::copy_constructed); // only lvalue-reference conversion operator currently is available
                S const r = std::move(static_cast< S const & >(v));
                CHECK (static_cast< S const & >(v).state_ == state::default_constructed);
                CHECK (r.state_ == state::copy_constructed); // `const &` operator win
            }
            {
                U v{};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::default_constructed);
                S const l{std::move(v)};
                CHECK (static_cast< S & >(v).state_ == state::default_constructed);
                CHECK (l.state_ == state::vcopy_constructed); // only lvalue-reference to const conversion operator currently is available
                S const r = std::move(static_cast< S & >(v));
                CHECK (static_cast< S & >(v).state_ == state::moved_from);
                CHECK (r.state_ == state::move_constructed);
            }
        }
        return true;
    }

    static
    bool
    constructible() noexcept
    {
        SA(std::is_copy_constructible_v< S >);
        SA(is_vcopy_constructible_v< S >    );
        SA(std::is_move_constructible_v< S >);
        SA(is_cmove_constructible_v< S >    );
        SA(std::is_copy_assignable_v< S >   );
        SA(is_vcopy_assignable_v< S >       );
        SA(std::is_move_assignable_v< S >   );
        SA(is_cmove_assignable_v< S >       );
        struct N {};
        SA(std::is_trivial_v< N >);
        {
            using U = variant< S, N >;
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                S const s{};
                CHECK (s.state_ == state::default_constructed);
                U const v{s};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S const & >(v).state_ == state::copy_constructed);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{s};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::vcopy_constructed);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S const s{};
                CHECK (s.state_ == state::default_constructed);
                U const v{std::move(s)};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S const & >(v).state_ == state::copy_constructed); // `const &` operator win
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{std::move(s)};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::move_constructed);
                CHECK (s.state_ == state::moved_from);
            }
        }
        {
            using U = variant< N, S >;
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                S const s{};
                CHECK (s.state_ == state::default_constructed);
                U const v{s};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S const & >(v).state_ == state::copy_constructed);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{s};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::vcopy_constructed);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S const s{};
                CHECK (s.state_ == state::default_constructed);
                U const v{std::move(s)};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S const & >(v).state_ == state::copy_constructed); // `const &` operator win
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{std::move(s)};
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::move_constructed);
                CHECK (s.state_ == state::moved_from);
            }
        }
        return true;
    }

    static
    bool
    assignable() noexcept
    {
        SA(std::is_copy_constructible_v< S >);
        SA(is_vcopy_constructible_v< S >    );
        SA(std::is_move_constructible_v< S >);
        SA(is_cmove_constructible_v< S >    );
        SA(std::is_copy_assignable_v< S >   );
        SA(is_vcopy_assignable_v< S >       );
        SA(std::is_move_assignable_v< S >   );
        SA(is_cmove_assignable_v< S >       );
        struct N {};
        SA(std::is_trivial_v< N >);
        {
            using U = variant< S, N >;
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                S const s{};
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< S >(v));
                v = s;
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::copy_assigned);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< S >(v));
                v = s;
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::vcopy_assigned);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S const s{};
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< S >(v));
                v = std::move(s);
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::copy_assigned);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< S >(v));
                v = std::move(s);
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::move_assigned);
                CHECK (s.state_ == state::moved_from);
            }
        }
        {
            using U = variant< N, S >;
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                S const s{};
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< N >(v));
                v = s;
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::copy_constructed);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< N >(v));
                v = s;
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::vcopy_constructed);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S const s{};
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< N >(v));
                v = std::move(s);
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::copy_constructed); // `const &` operator win
                CHECK (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< N >(v));
                v = std::move(s);
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::move_constructed);
                CHECK (s.state_ == state::moved_from);
            }
        }
        return true;
    }

    static
    bool
    in_place_constructible() noexcept
    {
        using ::versatile::in_place;
        using ::versatile::make_variant;
        struct X {};
        struct Y {};
        {
            struct A {};
            struct B {};
            using U = V< A, B >;
            U const a{in_place< A >};
            CHECK (is_active< A >(a));
            U const b{in_place< B >};
            CHECK (is_active< B >(b));
            auto const d = make_variant< U >();
            CHECK (is_active< A >(d));
        }
        {
            struct A { A() = delete; };
            struct B {};
            using U = V< A, B >;
            U const b{in_place< B >};
            CHECK (is_active< B >(b));
            auto const d = make_variant< U >();
            CHECK (is_active< B >(d));
        }
        {
            struct A { A(X) { ; } };
            struct B { B(Y) { ; } };
            using U = V< A, B >;
            U const a{in_place< A >, X{}};
            CHECK (is_active< A >(a));
            U const b{in_place< B >, Y{}};
            CHECK (is_active< B >(b));
            auto const x = make_variant< U >(X{});
            CHECK (is_active< A >(x));
            auto const y = make_variant< U >(Y{});
            CHECK (is_active< B >(y));
        }
        {
            struct A { A(X) { ; } };
            struct B { B(X) { ; } };
            using U = V< A, B >;
            U const a{in_place< A >, X{}};
            CHECK (is_active< A >(a));
            U const b{in_place< B >, X{}};
            CHECK (is_active< B >(b));
            auto const x = make_variant< U >(X{});
            CHECK (is_active< A >(x));
        }
        {
            struct B;
            struct A { A() = default; A(B &&) { ; } };
            struct B { B() = default; B(A) { ; } };
            using U = V< A, B >;
            U const a{in_place< A >, B{}};
            CHECK (is_active< A >(a));
            U const b{in_place< B >, A{}};
            CHECK (is_active< B >(b));
            auto const x = make_variant< U >(B{});
            CHECK (is_active< A >(x));
            auto const y = make_variant< U >(A{});
            CHECK (is_active< A >(y)); // move-constructed
        }
        {
            struct A { A(X, Y) { ; } };
            struct B { B(Y, X) { ; } };
            using U = V< A, B >;
            U const a{in_place< A >, X{}, Y{}};
            CHECK (is_active< A >(a));
            U const b{in_place< B >, Y{}, X{}};
            CHECK (is_active< B >(b));
            auto const x = make_variant< U >(X{}, Y{});
            CHECK (is_active< A >(x));
            auto const y = make_variant< U >(Y{}, X{});
            CHECK (is_active< B >(y));
        }
        return true;
    }

    static
    bool
    emplace() noexcept
    {
        using ::versatile::emplace;
        struct X {};
        struct Y {};
        struct Z {};
        {
            struct A { A(X) { ; } };
            struct B { B(Y) { ; } };
            using U = V< Z, A, B >;
            U v{};
            CHECK (is_active< Z >(v));
            emplace< A >(v, X{});
            CHECK (is_active< A >(v));
            emplace< B >(v, Y{});
            CHECK (is_active< B >(v));
            emplace< Z >(v);
            CHECK (is_active< Z >(v));
        }
        {
            struct A { A(X) { ; } };
            struct B { B(X) { ; } };
            using U = V< Z, A, B >;
            U v{};
            CHECK (is_active< Z >(v));
            emplace< A >(v, X{});
            CHECK (is_active< A >(v));
            emplace< B >(v, X{});
            CHECK (is_active< B >(v));
            emplace< Z >(v);
            CHECK (is_active< Z >(v));
        }
        {
            struct B;
            struct A { A() = default; A(B &&) { ; } };
            struct B { B() = default; B(A) { ; } };
            using U = V< Z, A, B >;
            U v{};
            CHECK (is_active< Z >(v));
            emplace< A >(v, B{});
            CHECK (is_active< A >(v));
            emplace< B >(v, A{});
            CHECK (is_active< B >(v));
            emplace< Z >(v);
            CHECK (is_active< Z >(v));
        }
        {
            struct A { A(X, Y) { ; } };
            struct B { B(Y, X) { ; } };
            using U = V< Z, A, B >;
            U v{};
            CHECK (is_active< Z >(v));
            emplace< A >(v, X{}, Y{});
            CHECK (is_active< A >(v));
            emplace< B >(v, Y{}, X{});
            CHECK (is_active< B >(v));
            emplace< Z >(v);
            CHECK (is_active< Z >(v));
        }
        return true;
    }

public :

    static
    bool
    run() noexcept
    {
        CHECK (default_constructible());
        CHECK (copy_move_constructible());
        CHECK (copy_move_assignable());
        CHECK (convertible()); // conversion
        CHECK (constructible()); // conversion
        CHECK (assignable()); // conversion
        CHECK (in_place_constructible());
        CHECK (emplace());
        return true;
    }

};

} // namespace test
