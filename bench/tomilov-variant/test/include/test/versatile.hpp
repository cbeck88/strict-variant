#pragma once

#include "test/prologue.hpp"

#include <versatile/versatile.hpp>
#include <versatile/utility.hpp>

#include <type_traits>
#include <utility>
#include <string>
#include <vector>
#include <deque>
#include <list>

namespace test
{

using ::versatile::is_active;

template< template< typename ... > class wrapper,
          template< typename ... > class versatile >
class check_invariants
{

    template< typename ...types >
    using V = versatile< typename wrapper< types >::type... >;

    struct trivial
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            //S(S &) = default; // ?sould not be user-declared to be trivial
            //S(S const &&) { ; } // ?sould not be user-declared/provided to be trivial
            S(S &&) = default;

            S & operator = (S const &) = default;
            //S & operator = (S &) = default; // ?sould not be user-declared to be trivial
            //S & operator = (S const &&) { return *this; } // ?sould not be user-declared/provided to be trivial
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_trivial_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);
        SA(std::is_trivially_copyable_v< U >);

        SA(std::is_standard_layout_v< S >);
        SA(std::is_standard_layout_v< U >);

        SA(std::is_literal_type_v< S >); // constexpr defaulted default c-tor
        SA(std::is_literal_type_v< U >);

        SA(std::is_pod_v< S >);
        SA(!std::is_pod_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct trivially_copyable
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() { ; }

            S(S const &) = default;
            //S(S &) = default; // ?sould not be user-declared to be trivially copyable
            //S(S const &&) { ; } // ?sould not be user-declared/provided to be trivially copyable
            S(S &&) = default;

            S & operator = (S const &) = default;
            //S & operator = (S &) = default; // ?sould not be user-declared to be trivially copyable
            //S & operator = (S const &&) { return *this; } // ?sould not be user-declared/provided to be trivially copyable
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(!std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_trivially_copyable_v< S >);
        SA(std::is_trivially_copyable_v< U >);

        SA(std::is_standard_layout_v< S >);
        SA(std::is_standard_layout_v< U >);

        SA(!std::is_literal_type_v< S >);
        SA(!std::is_literal_type_v< U >);

        SA(!std::is_pod_v< S >);
        SA(!std::is_pod_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct default_constructor
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() { ; }

            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(!std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(std::is_trivially_copy_constructible_v< S >);
        SA(std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(is_trivially_vcopy_constructible_v< U >);

        SA(std::is_trivially_move_constructible_v< S >);
        SA(std::is_trivially_move_constructible_v< U >);
        SA(is_trivially_cmove_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct destructor
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() { ; }

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(!std::is_trivially_default_constructible_v< S >); // ! false
        SA(!std::is_trivially_default_constructible_v< U >); // !

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(!std::is_trivially_destructible_v< S >);
        SA(!std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(!std::is_trivially_copy_constructible_v< S >); // ? false
        SA(!std::is_trivially_copy_constructible_v< U >); // ?
        SA(!is_trivially_vcopy_constructible_v< S >); // ?
        SA(!is_trivially_vcopy_constructible_v< U >); // ?

        SA(!std::is_trivially_move_constructible_v< S >); // ? false
        SA(!std::is_trivially_move_constructible_v< U >); // ?
        SA(!is_trivially_cmove_constructible_v< S >); // ?
        SA(!is_trivially_cmove_constructible_v< U >); // ?

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct copy_constructor
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) { ; }
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(!std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(!is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(!is_cmove_constructible_v< U >);

        SA(!std::is_trivially_copy_constructible_v< S >);
        SA(!std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(!is_trivially_vcopy_constructible_v< U >);

        SA(std::is_trivially_move_constructible_v< S >);
        SA(std::is_trivially_move_constructible_v< U >);
        SA(!is_trivially_cmove_constructible_v< S >);
        SA(!is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >); // ?
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct vcopy_constructor
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            S(S &) { ; }
            //S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(std::is_trivially_copy_constructible_v< S >);
        SA(std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(is_trivially_vcopy_constructible_v< U >);

        SA(std::is_trivially_move_constructible_v< S >);
        SA(std::is_trivially_move_constructible_v< U >);
        SA(is_trivially_cmove_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct move_constructor
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) { ; }

            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(std::is_trivially_copy_constructible_v< S >);
        SA(std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(is_trivially_vcopy_constructible_v< U >);

        SA(!std::is_trivially_move_constructible_v< S >);
        SA(!std::is_trivially_move_constructible_v< U >);
        SA(is_trivially_cmove_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct cmove_constructor
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            S(S &) = default;
            /*!*/S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(std::is_trivially_copy_constructible_v< S >);
        SA(std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(is_trivially_vcopy_constructible_v< U >);

        SA(std::is_trivially_move_constructible_v< S >);
        SA(std::is_trivially_move_constructible_v< U >);
        SA(!is_trivially_cmove_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct copy_assignment
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) { ; }
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(std::is_trivially_copy_constructible_v< S >);
        SA(std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(is_trivially_vcopy_constructible_v< U >);

        SA(std::is_trivially_move_constructible_v< S >);
        SA(std::is_trivially_move_constructible_v< U >);
        SA(is_trivially_cmove_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(!std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(!is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(!is_cmove_assignable_v< U >);

        SA(!std::is_trivially_copy_assignable_v< S >);
        SA(!std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(!is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(!is_trivially_cmove_assignable_v< S >);
        SA(!is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct vcopy_assignment
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) = default;
            S & operator = (S &) { ; }
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(std::is_trivially_copy_constructible_v< S >);
        SA(std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(is_trivially_vcopy_constructible_v< U >);

        SA(std::is_trivially_move_constructible_v< S >);
        SA(std::is_trivially_move_constructible_v< U >);
        SA(is_trivially_cmove_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct move_assignment
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            //S & operator = (S const &&) { return *this; }
            S & operator = (S &&) { return *this; }

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(std::is_trivially_copy_constructible_v< S >);
        SA(std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(is_trivially_vcopy_constructible_v< U >);

        SA(std::is_trivially_move_constructible_v< S >);
        SA(std::is_trivially_move_constructible_v< U >);
        SA(is_trivially_cmove_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(!std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    struct cmove_assignment
    {

        struct S
        {

            int i;
            S(int j) : i(j) { ; }

            S() = default;

            S(S const &) = default;
            S(S &) = default;
            //S(S const &&) { ; }
            S(S &&) = default;

            S & operator = (S const &) = default;
            S & operator = (S &) = default;
            /*!*/S & operator = (S const &&) { return *this; }
            S & operator = (S &&) = default;

            ~S() = default;

        };

        using U = V< S >;

        SA(std::is_default_constructible_v< S >);
        SA(std::is_default_constructible_v< U >);
        SA(std::is_trivially_default_constructible_v< S >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< S >);
        SA(std::is_destructible_v< U >);
        SA(std::is_trivially_destructible_v< S >);
        SA(std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< S >);
        SA(std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< S >);
        SA(is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< S >);
        SA(std::is_move_constructible_v< U >);
        SA(is_cmove_constructible_v< S >);
        SA(is_cmove_constructible_v< U >);

        SA(std::is_trivially_copy_constructible_v< S >);
        SA(std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< S >);
        SA(is_trivially_vcopy_constructible_v< U >);

        SA(std::is_trivially_move_constructible_v< S >);
        SA(std::is_trivially_move_constructible_v< U >);
        SA(is_trivially_cmove_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< S >);
        SA(std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< S >);
        SA(is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< S >);
        SA(std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< S >);
        SA(is_cmove_assignable_v< U >);

        SA(std::is_trivially_copy_assignable_v< S >);
        SA(std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< S >);
        SA(is_trivially_vcopy_assignable_v< U >);

        SA(std::is_trivially_move_assignable_v< S >);
        SA(std::is_trivially_move_assignable_v< U >);
        SA(!is_trivially_cmove_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

    template< typename C >
    struct container
    {

        using U = V< C >;

        SA(std::is_default_constructible_v< C >);
        SA(std::is_default_constructible_v< U >);
        SA(!std::is_trivially_default_constructible_v< C >);
        SA(!std::is_trivially_default_constructible_v< U >);

        SA(std::is_destructible_v< C >);
        SA(std::is_destructible_v< U >);
        SA(!std::is_trivially_destructible_v< C >);
        SA(!std::is_trivially_destructible_v< U >);

        SA(std::is_copy_constructible_v< C >);
        SA(!std::is_copy_constructible_v< U >);
        SA(is_vcopy_constructible_v< C >);
        SA(!is_vcopy_constructible_v< U >);

        SA(std::is_move_constructible_v< C >);
        SA(std::is_move_constructible_v< U >); // ?
        SA(is_cmove_constructible_v< C >);
        SA(!is_cmove_constructible_v< U >);

        SA(!std::is_trivially_copy_constructible_v< C >);
        SA(!std::is_trivially_copy_constructible_v< U >);
        SA(!is_trivially_vcopy_constructible_v< C >);
        SA(!is_trivially_vcopy_constructible_v< U >);

        SA(!std::is_trivially_move_constructible_v< C >);
        SA(!std::is_trivially_move_constructible_v< U >);
        SA(!is_trivially_cmove_constructible_v< C >);
        SA(!is_trivially_cmove_constructible_v< U >);

        SA(std::is_copy_assignable_v< C >);
        SA(!std::is_copy_assignable_v< U >);
        SA(is_vcopy_assignable_v< C >);
        SA(!is_vcopy_assignable_v< U >);

        SA(std::is_move_assignable_v< C >);
        SA(!std::is_move_assignable_v< U >);
        SA(is_cmove_assignable_v< C >);
        SA(!is_cmove_assignable_v< U >);

        SA(!std::is_trivially_copy_assignable_v< C >);
        SA(!std::is_trivially_copy_assignable_v< U >);
        SA(!is_trivially_vcopy_assignable_v< C >);
        SA(!is_trivially_vcopy_assignable_v< U >);

        SA(!std::is_trivially_move_assignable_v< C >);
        SA(!std::is_trivially_move_assignable_v< U >);
        SA(!is_trivially_cmove_assignable_v< C >);
        SA(!is_trivially_cmove_assignable_v< U >);

        constexpr
        static
        bool
        run() noexcept
        {
            return true;
        }

    };

public :

    constexpr
    static
    bool
    run() noexcept // just for implicit instantiation
    {
        { // all std::containers are identical
            using S = common_type<>;
            SA(container< std::string >::run());
            SA(container< std::vector< S > >::run());
            SA(container< std::deque< S > >::run());
            SA(container< std::list< S > >::run());
        }
        SA(trivial                ::run());
        SA(trivially_copyable     ::run());
        SA(default_constructor    ::run());
        SA(destructor             ::run());
        SA(copy_constructor       ::run());
        SA(vcopy_constructor      ::run());
        SA(move_constructor       ::run());
        SA(cmove_constructor      ::run());
        SA(copy_assignment        ::run());
        SA(vcopy_assignment       ::run());
        SA(move_assignment        ::run());
        SA(cmove_assignment       ::run());
        return true;
    }

};

template< template< typename ... > class wrapper,
          template< typename ... > class versatile >
class check_triviality
{

    template< typename ...types >
    using V = versatile< typename wrapper< types >::type... >;

    CONSTEXPRF
    static
    bool
    trivially_default_constructible() noexcept
    {
        struct S
        {
            CONSTEXPRF S() = default;
            CONSTEXPRF S(S const &) { ; }
            CONSTEXPRF S(S &) { ; }
            CONSTEXPRF S(S const &&) { ; }
            CONSTEXPRF S(S &&) { ; }
            CONSTEXPRF S & operator = (S const &) { return *this; }
            CONSTEXPRF S & operator = (S &) { return *this; }
            CONSTEXPRF S & operator = (S const &&) { return *this; }
            CONSTEXPRF S & operator = (S &&) { return *this; }
            //~S() DESTRUCTOR
        };
        SA(std::is_trivially_default_constructible_v< S >);
        struct N {};
        SA(std::is_trivially_default_constructible_v< N >);
        {
            using U = V< S, N >;
            SA(!std::is_trivially_default_constructible_v< U >);
            {
                CONSTEXPR U v{};
                ASSERT ( is_active< S >(v));
                ASSERT (!is_active< N >(v)); // check is_active itself
            }
            {
                U v{};
                CHECK ( is_active< S >(v));
                CHECK (!is_active< N >(v));
            }
        }
        {
            using U = V< N, S >;
            SA(!std::is_trivially_default_constructible_v< U >);
            {
                CONSTEXPR U v{};
                ASSERT ( is_active< N >(v));
                ASSERT (!is_active< S >(v));
            }
            {
                U v{};
                CHECK ( is_active< N >(v));
                CHECK (!is_active< S >(v));
            }
        }
        return true;
    }

    CONSTEXPRF
    static
    bool
    trivially_copy_move_constructible() noexcept
    {
        struct S
        {
            CONSTEXPRF S() { ; }
            CONSTEXPRF S(S const &) = default;
            CONSTEXPRF S(S &) = default;
            //CONSTEXPRF S(S const &&) { ; }
            CONSTEXPRF S(S &&) = default;
            CONSTEXPRF S & operator = (S const &) { return *this; }
            CONSTEXPRF S & operator = (S &) { return *this; }
            //CONSTEXPRF S & operator = (S const &&) { return *this; }
            CONSTEXPRF S & operator = (S &&) { return *this; }
            ~S() = default;
        };
        SA(std::is_trivially_copy_constructible_v< S >);
        SA(!is_trivially_vcopy_constructible_v< S >   ); // ?
        SA(std::is_trivially_move_constructible_v< S >);
        SA(is_trivially_cmove_constructible_v< S >    );
        SA(std::is_copy_constructible_v< S >          );
        SA(is_vcopy_constructible_v< S >              );
        SA(std::is_move_constructible_v< S >          );
        SA(is_cmove_constructible_v< S >              );
        struct N {};
        SA(std::is_trivial_v< N >);
        {
            using U = V< S, N >;
            SA(std::is_trivially_copy_constructible_v< U >);
            SA(is_trivially_vcopy_constructible_v< U >);
            SA(std::is_trivially_move_constructible_v< U >);
            SA(is_trivially_cmove_constructible_v< U >);
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            {
                CONSTEXPR U v{N{}};
                ASSERT (is_active< N >(v));
                CONSTEXPR U w{v};
                ASSERT (is_active< N >(w));
            }
            {
                CONSTEXPR U v{S{}};
                ASSERT (is_active< S >(v));
                CONSTEXPR U w{v};
                ASSERT (is_active< S >(w));
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
                CONSTEXPR U v{N{}};
                ASSERT (is_active< N >(v));
                CONSTEXPR U w{std::move(v)};
                ASSERT (is_active< N >(w));
            }
            {
                CONSTEXPR U v{S{}};
                ASSERT (is_active< S >(v));
                CONSTEXPR U w{std::move(v)};
                ASSERT (is_active< S >(w));
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
            using U = V< N, S >;
            SA(std::is_trivially_copy_constructible_v< U >);
            SA(is_trivially_vcopy_constructible_v< U >);
            SA(std::is_trivially_move_constructible_v< U >);
            SA(is_trivially_cmove_constructible_v< U >);
            SA(std::is_copy_constructible_v< U >);
            SA(is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >);
            SA(is_cmove_constructible_v< U >);
            {
                CONSTEXPR U v{N{}};
                ASSERT (is_active< N >(v));
                CONSTEXPR U w{v};
                ASSERT (is_active< N >(w));
            }
            {
                CONSTEXPR U v{S{}};
                ASSERT (is_active< S >(v));
                CONSTEXPR U w{v};
                ASSERT (is_active< S >(w));
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
                CONSTEXPR U v{N{}};
                ASSERT (is_active< N >(v));
                CONSTEXPR U w{std::move(v)};
                ASSERT (is_active< N >(w));
            }
            {
                CONSTEXPR U v{S{}};
                ASSERT (is_active< S >(v));
                CONSTEXPR U w{std::move(v)};
                ASSERT (is_active< S >(w));
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

    CONSTEXPRF
    static
    bool
    trivially_copy_move_assignable() noexcept
    {
        struct S
        {
            CONSTEXPRF S() { ; }
            CONSTEXPRF S(S const &) = default;
            CONSTEXPRF S(S &) = default;
            //CONSTEXPRF S(S const &&) { ; }
            CONSTEXPRF S(S &&) = default;
            CONSTEXPRF S & operator = (S const &) = default;
            CONSTEXPRF S & operator = (S &) = default;
            //CONSTEXPRF S & operator = (S const &&) { return *this; }
            CONSTEXPRF S & operator = (S &&) = default;
            //~S() DESTRUCTOR // clang bug: if destructor is user-declared and defaulted, then defaulted assignment operators become non-trivial and marked as non-constexpr
        };
        SA(std::is_trivially_copy_assignable_v< S >);
        SA(!is_trivially_vcopy_assignable_v< S >   ); // ?
        SA(std::is_trivially_move_assignable_v< S >);
        SA(is_trivially_cmove_assignable_v< S >    );
        SA(std::is_copy_assignable_v< S >          );
        SA(is_vcopy_assignable_v< S >              );
        SA(std::is_move_assignable_v< S >          );
        SA(is_cmove_assignable_v< S >              );
        struct N {};
        SA(std::is_trivial_v< N >);
        {
            using U = V< S, N >;
            SA(std::is_trivially_copy_assignable_v< U >);
            SA(is_trivially_vcopy_assignable_v< U >);
            SA(std::is_trivially_move_assignable_v< U >);
            SA(is_trivially_cmove_assignable_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                CONSTEXPR U v{N{}};
                ASSERT (is_active< N >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = v;
                CHECK (is_active< N >(w));
            }
            {
                CONSTEXPR U v{S{}};
                ASSERT (is_active< S >(v));
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
                CONSTEXPR U v{N{}};
                ASSERT (is_active< N >(v));
                U w{};
                CHECK (is_active< S >(w));
                w = std::move(v);
                CHECK (is_active< N >(w));
            }
            {
                CONSTEXPR U v{S{}};
                ASSERT (is_active< S >(v));
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
            using U = V< N, S >;
            SA(std::is_trivially_copy_assignable_v< U >);
            SA(is_trivially_vcopy_assignable_v< U >);
            SA(std::is_trivially_move_assignable_v< U >);
            SA(is_trivially_cmove_assignable_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                CONSTEXPR U v{N{}};
                ASSERT (is_active< N >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = v;
                CHECK (is_active< N >(w));
            }
            {
                CONSTEXPR U v{S{}};
                ASSERT (is_active< S >(v));
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
                CONSTEXPR U v{N{}};
                ASSERT (is_active< N >(v));
                U w{};
                CHECK (is_active< N >(w));
                w = std::move(v);
                CHECK (is_active< N >(w));
            }
            {
                CONSTEXPR U v{S{}};
                ASSERT (is_active< S >(v));
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

    CONSTEXPRF
    static
    bool
    convertible() noexcept
    {
        struct S
        {
            state state_ = state::never_used;
            CONSTEXPRF S() : state_{state::default_constructed} { ; }
            CONSTEXPRF S(S const &) : state_{state::copy_constructed} { ; }
            CONSTEXPRF S(S &) : state_{state::vcopy_constructed} { ; }
            //CONSTEXPRF S(S const &&) { state_ = state::cmove_constructed; }
            CONSTEXPRF S(S && s) : state_{state::move_constructed} { s.state_ = state::moved_from; }
            CONSTEXPRF S & operator = (S const &) { state_ = state::copy_assigned; return *this; }
            CONSTEXPRF S & operator = (S &) { state_ = state::vcopy_assigned; return *this; }
            //CONSTEXPRF S & operator = (S const &&) { state_ = state::cmove_assigned; return *this; }
            CONSTEXPRF S & operator = (S && s) { state_ = state::move_assigned; s.state_ = state::moved_from; return *this; }
            ~S() DESTRUCTOR
        };
        SA(!std::is_trivially_copy_constructible_v< S >);
        SA(!is_trivially_vcopy_constructible_v< S >    );
        SA(!std::is_trivially_move_constructible_v< S >);
        SA(!is_trivially_cmove_constructible_v< S >    );
        SA(std::is_copy_constructible_v< S >           );
        SA(is_vcopy_constructible_v< S >               );
        SA(std::is_move_constructible_v< S >           );
        SA(is_cmove_constructible_v< S >               );
        SA(!std::is_trivially_copy_assignable_v< S >   );
        SA(!is_trivially_vcopy_assignable_v< S >       );
        SA(!std::is_trivially_move_assignable_v< S >   );
        SA(!is_trivially_cmove_assignable_v< S >       );
        SA(std::is_copy_assignable_v< S >              );
        SA(is_vcopy_assignable_v< S >                  );
        SA(std::is_move_assignable_v< S >              );
        SA(is_cmove_assignable_v< S >                  );
        {
            using U = V< S >;
            SA(!std::is_trivially_copy_constructible_v< U >);
            SA(!is_trivially_vcopy_constructible_v< U >);
            SA(!std::is_trivially_move_constructible_v< U >);
            SA(!is_trivially_cmove_constructible_v< U >);
            SA(!std::is_copy_constructible_v< U >);
            SA(!is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >); // ?
            SA(!is_cmove_constructible_v< U >);
            SA(!std::is_trivially_copy_assignable_v< U >);
            SA(!is_trivially_vcopy_assignable_v< U >);
            SA(!std::is_trivially_move_assignable_v< U >);
            SA(!is_trivially_cmove_assignable_v< U >);
            SA(!std::is_copy_assignable_v< U >);
            SA(!is_vcopy_assignable_v< U >);
            SA(!std::is_move_assignable_v< U >);
            SA(!is_cmove_assignable_v< U >);
            SA(!is_convertible_v< U, S >);
            SA(!is_convertible_v< U const, S >);
            SA(!is_convertible_v< U &, S >);
            SA(!is_convertible_v< U const &, S >);
            SA(is_explicitly_convertible_v< U, S >);
            SA(is_explicitly_convertible_v< U const, S >);
            SA(is_explicitly_convertible_v< U &, S >);
            SA(is_explicitly_convertible_v< U const &, S >);
            SA(is_convertible_v< S, U >);
            SA(is_convertible_v< S const, U >);
            SA(is_convertible_v< S &, U >);
            SA(is_convertible_v< S const &, U >);
            {
                struct T {};
                SA(!is_explicitly_convertible_v< U, T >); // SFINAE-disabled conversions
                SA(!is_explicitly_convertible_v< U const, T >); // SFINAE-disabled conversions
                SA(!is_explicitly_convertible_v< U &, T >); // SFINAE-disabled conversions
                SA(!is_explicitly_convertible_v< U const &, T >); // SFINAE-disabled conversions
            }
            {
                CONSTEXPR U v{};
                ASSERT (is_active< S >(v));
                ASSERT (static_cast< S const & >(v).state_ == state::default_constructed);
                CONSTEXPR S s{v};
                ASSERT (static_cast< S const & >(v).state_ == state::default_constructed);
                ASSERT (s.state_ == state::copy_constructed);
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
                CONSTEXPR U v{};
                ASSERT (is_active< S >(v));
                ASSERT (static_cast< S const & >(v).state_ == state::default_constructed);
                CONSTEXPR S l{std::move(v)};
                ASSERT (static_cast< S const & >(v).state_ == state::default_constructed);
                ASSERT (l.state_ == state::copy_constructed); // only lvalue-reference conversion operator currently is available
                CONSTEXPR S r = std::move(static_cast< S const & >(v));
                ASSERT (static_cast< S const & >(v).state_ == state::default_constructed);
                ASSERT (r.state_ == state::copy_constructed); // `const &` operator win
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

    CONSTEXPRF
    static
    bool
    constructible() noexcept
    {
        struct S
        {
            state state_ = state::never_used;
            CONSTEXPRF S() : state_{state::default_constructed} { ; }
            CONSTEXPRF S(S const &) : state_{state::copy_constructed} { ; }
            CONSTEXPRF S(S &) : state_{state::vcopy_constructed} { ; }
            //CONSTEXPRF S(S const &&) { state_ = state::cmove_constructed; }
            CONSTEXPRF S(S && s) : state_{state::move_constructed} { s.state_ = state::moved_from; }
            CONSTEXPRF S & operator = (S const &) { state_ = state::copy_assigned; return *this; }
            CONSTEXPRF S & operator = (S &) { state_ = state::vcopy_assigned; return *this; }
            //CONSTEXPRF S & operator = (S const &&) { state_ = state::cmove_assigned; return *this; }
            CONSTEXPRF S & operator = (S && s) { state_ = state::move_assigned; s.state_ = state::moved_from; return *this; }
            ~S() DESTRUCTOR
        };
        SA(!std::is_trivially_copy_constructible_v< S >);
        SA(!is_trivially_vcopy_constructible_v< S >    ); // ?
        SA(!std::is_trivially_move_constructible_v< S >);
        SA(!is_trivially_cmove_constructible_v< S >    );
        SA(std::is_copy_constructible_v< S >           );
        SA(is_vcopy_constructible_v< S >               );
        SA(std::is_move_constructible_v< S >           );
        SA(is_cmove_constructible_v< S >               );
        SA(!std::is_trivially_copy_assignable_v< S >   );
        SA(!is_trivially_vcopy_assignable_v< S >       ); // ?
        SA(!std::is_trivially_move_assignable_v< S >   );
        SA(!is_trivially_cmove_assignable_v< S >       );
        SA(std::is_copy_assignable_v< S >              );
        SA(is_vcopy_assignable_v< S >                  );
        SA(std::is_move_assignable_v< S >              );
        SA(is_cmove_assignable_v< S >                  );
        struct N {};
        SA(std::is_trivial_v< N >);
        {
            using U = V< S, N >;
            SA(!std::is_trivially_copy_constructible_v< U >);
            SA(!is_trivially_vcopy_constructible_v< U >); // ?
            SA(!std::is_trivially_move_constructible_v< U >);
            SA(!is_trivially_cmove_constructible_v< U >);
            SA(!std::is_copy_constructible_v< U >);
            SA(!is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >); // ?
            SA(!is_cmove_constructible_v< U >);
            SA(!std::is_trivially_copy_assignable_v< U >);
            SA(!is_trivially_vcopy_assignable_v< U >); // ?
            SA(!std::is_trivially_move_assignable_v< U >);
            SA(!is_trivially_cmove_assignable_v< U >);
            SA(!std::is_copy_assignable_v< U >);
            SA(!is_vcopy_assignable_v< U >);
            SA(!std::is_move_assignable_v< U >);
            SA(!is_cmove_assignable_v< U >);
            {
                CONSTEXPR S s{};
                ASSERT (s.state_ == state::default_constructed);
                CONSTEXPR U v{s};
                ASSERT (is_active< S >(v));
                ASSERT (static_cast< S const & >(v).state_ == state::copy_constructed);
                ASSERT (s.state_ == state::default_constructed);
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
                CONSTEXPR S s{};
                ASSERT (s.state_ == state::default_constructed);
                CONSTEXPR U v{std::move(s)};
                ASSERT (is_active< S >(v));
                ASSERT (static_cast< S const & >(v).state_ == state::copy_constructed); // `const &` operator win
                ASSERT (s.state_ == state::default_constructed);
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
            using U = V< N, S >;
            SA(!std::is_trivially_copy_constructible_v< U >);
            SA(!is_trivially_vcopy_constructible_v< U >); // ?
            SA(!std::is_trivially_move_constructible_v< U >);
            SA(!is_trivially_cmove_constructible_v< U >);
            SA(!std::is_copy_constructible_v< U >);
            SA(!is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >); // ?
            SA(!is_cmove_constructible_v< U >);
            SA(!std::is_trivially_copy_assignable_v< U >);
            SA(!is_trivially_vcopy_assignable_v< U >); // ?
            SA(!std::is_trivially_move_assignable_v< U >);
            SA(!is_trivially_cmove_assignable_v< U >);
            SA(!std::is_copy_assignable_v< U >);
            SA(!is_vcopy_assignable_v< U >);
            SA(!std::is_move_assignable_v< U >);
            SA(!is_cmove_assignable_v< U >);
            {
                CONSTEXPR S s{};
                ASSERT (s.state_ == state::default_constructed);
                CONSTEXPR U v{s};
                ASSERT (is_active< S >(v));
                ASSERT (static_cast< S const & >(v).state_ == state::copy_constructed);
                ASSERT (s.state_ == state::default_constructed);
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
                CONSTEXPR S s{};
                ASSERT (s.state_ == state::default_constructed);
                CONSTEXPR U v{std::move(s)};
                ASSERT (is_active< S >(v));
                ASSERT (static_cast< S const & >(v).state_ == state::copy_constructed); // `const &` operator win
                ASSERT (s.state_ == state::default_constructed);
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

    CONSTEXPRF
    static
    bool
    assignable() noexcept
    {
        struct S
        {
            state state_ = state::never_used;
            CONSTEXPRF S() : state_{state::default_constructed} { ; }
            CONSTEXPRF S(S const &) : state_{state::copy_constructed} { ; }
            CONSTEXPRF S(S &) : state_{state::vcopy_constructed} { ; }
            //CONSTEXPRF S(S const &&) { state_ = state::cmove_constructed; }
            CONSTEXPRF S(S && s) : state_{state::move_constructed} { s.state_ = state::moved_from; }
            CONSTEXPRF S & operator = (S const &) = default;
            CONSTEXPRF S & operator = (S &) = default;
            //CONSTEXPRF S & operator = (S const &&) { return *this; }
            CONSTEXPRF S & operator = (S &&) = default;
            //~S() DESTRUCTOR // clang bug: if destructor is user-declared and defaulted, then defaulted assignment operators become non-trivial and marked as non-constexpr
        };
        SA(!std::is_trivially_copy_constructible_v< S >);
        SA(!is_trivially_vcopy_constructible_v< S >    );
        SA(!std::is_trivially_move_constructible_v< S >);
        SA(!is_trivially_cmove_constructible_v< S >    );
        SA(std::is_copy_constructible_v< S >           );
        SA(is_vcopy_constructible_v< S >               );
        SA(std::is_move_constructible_v< S >           );
        SA(is_cmove_constructible_v< S >               );
        SA(std::is_trivially_copy_assignable_v< S >    );
        SA(!is_trivially_vcopy_assignable_v< S >       ); // ?
        SA(std::is_trivially_move_assignable_v< S >    );
        SA(is_trivially_cmove_assignable_v< S >        );
        SA(std::is_copy_assignable_v< S >              );
        SA(is_vcopy_assignable_v< S >                  );
        SA(std::is_move_assignable_v< S >              );
        SA(is_cmove_assignable_v< S >                  );
        struct N {};
        SA(std::is_trivial_v< N >);
        {
            using U = V< S, N >;
            SA(!std::is_trivially_copy_constructible_v< U >);
            SA(!is_trivially_vcopy_constructible_v< U >); // ?
            SA(!std::is_trivially_move_constructible_v< U >);
            SA(!is_trivially_cmove_constructible_v< U >);
            SA(!std::is_copy_constructible_v< U >);
            SA(!is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >); // ?
            SA(!is_cmove_constructible_v< U >);
            SA(std::is_trivially_copy_assignable_v< U >);
            SA(is_trivially_vcopy_assignable_v< U >);
            SA(std::is_trivially_move_assignable_v< U >);
            SA(is_trivially_cmove_assignable_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                CONSTEXPR S s{};
                ASSERT (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< S >(v));
                v = s;
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::copy_constructed);
                ASSERT (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< S >(v));
                v = s;
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::vcopy_constructed);
                CHECK (s.state_ == state::default_constructed);
            }
            {
                CONSTEXPR S s{};
                ASSERT (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< S >(v));
                v = std::move(s);
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::copy_constructed); // `const &` operator win
                ASSERT (s.state_ == state::default_constructed);
            }
            {
                S s;
                CHECK (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< S >(v));
                v = std::move(s);
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::move_constructed);
                CHECK (s.state_ == state::moved_from);
            }
        }
        {
            using U = V< N, S >;
            SA(!std::is_trivially_copy_constructible_v< U >);
            SA(!is_trivially_vcopy_constructible_v< U >);
            SA(!std::is_trivially_move_constructible_v< U >);
            SA(!is_trivially_cmove_constructible_v< U >);
            SA(!std::is_copy_constructible_v< U >);
            SA(!is_vcopy_constructible_v< U >);
            SA(std::is_move_constructible_v< U >); // ?
            SA(!is_cmove_constructible_v< U >);
            SA(std::is_trivially_copy_assignable_v< U >);
            SA(is_trivially_vcopy_assignable_v< U >);
            SA(std::is_trivially_move_assignable_v< U >);
            SA(is_trivially_cmove_assignable_v< U >);
            SA(std::is_copy_assignable_v< U >);
            SA(is_vcopy_assignable_v< U >);
            SA(std::is_move_assignable_v< U >);
            SA(is_cmove_assignable_v< U >);
            {
                CONSTEXPR S s{};
                ASSERT (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< N >(v));
                v = s;
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::copy_constructed);
                ASSERT (s.state_ == state::default_constructed);
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
                CONSTEXPR S s{};
                ASSERT (s.state_ == state::default_constructed);
                U v{};
                CHECK (is_active< N >(v));
                v = std::move(s);
                CHECK (is_active< S >(v));
                CHECK (static_cast< S & >(v).state_ == state::copy_constructed); // `const &` operator win
                ASSERT (s.state_ == state::default_constructed);
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

    CONSTEXPRF
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
            CONSTEXPR U a{in_place< A >};
            ASSERT (is_active< A >(a));
            CONSTEXPR U b{in_place< B >};
            ASSERT (is_active< B >(b));
            //CONSTEXPR auto d = make_variant< U >();
            //ASSERT (is_active< A >(d));
        }
        {
            struct A { A() = delete; };
            struct B {};
            using U = V< A, B >;
            CONSTEXPR U b{in_place< B >};
            ASSERT (is_active< B >(b));
            CONSTEXPR auto d = make_variant< U >();
            ASSERT (is_active< B >(d));
        }
        {
            struct A { CONSTEXPRF A(X) { ; } };
            struct B { CONSTEXPRF B(Y) { ; } };
            using U = V< A, B >;
            CONSTEXPR U a{in_place< A >, X{}};
            ASSERT (is_active< A >(a));
            CONSTEXPR U b{in_place< B >, Y{}};
            ASSERT (is_active< B >(b));
            CONSTEXPR auto x = make_variant< U >(X{});
            ASSERT (is_active< A >(x));
            CONSTEXPR auto y = make_variant< U >(Y{});
            ASSERT (is_active< B >(y));
        }
        {
            struct A { CONSTEXPRF A(X) { ; } };
            struct B { CONSTEXPRF B(X) { ; } };
            using U = V< A, B >;
            CONSTEXPR U a{in_place< A >, X{}};
            ASSERT (is_active< A >(a));
            CONSTEXPR U b{in_place< B >, X{}};
            ASSERT (is_active< B >(b));
            CONSTEXPR auto x = make_variant< U >(X{});
            ASSERT (is_active< A >(x));
        }
        {
            struct B;
            struct A { CONSTEXPRF A() = default; CONSTEXPRF A(B &&) { ; } };
            struct B { CONSTEXPRF B() = default; CONSTEXPRF B(A) { ; } };
            using U = V< A, B >;
            CONSTEXPR U a{in_place< A >, B{}};
            ASSERT (is_active< A >(a));
            CONSTEXPR U b{in_place< B >, A{}};
            ASSERT (is_active< B >(b));
            CONSTEXPR auto x = make_variant< U >(B{});
            ASSERT (is_active< A >(x));
            CONSTEXPR auto y = make_variant< U >(A{});
            ASSERT (is_active< A >(y)); // move-constructed
        }
        {
            struct A { CONSTEXPRF A(X, Y) { ; } };
            struct B { CONSTEXPRF B(Y, X) { ; } };
            using U = V< A, B >;
            CONSTEXPR U a{in_place< A >, X{}, Y{}};
            ASSERT (is_active< A >(a));
            CONSTEXPR U b{in_place< B >, Y{}, X{}};
            ASSERT (is_active< B >(b));
            CONSTEXPR auto x = make_variant< U >(X{}, Y{});
            ASSERT (is_active< A >(x));
            CONSTEXPR auto y = make_variant< U >(Y{}, X{});
            ASSERT (is_active< B >(y));
        }
        return true;
    }

    CONSTEXPRF
    static
    bool
    emplace() noexcept
    {
        using ::versatile::emplace;
        struct X {};
        struct Y {};
        struct Z {};
        {
            struct A { CONSTEXPRF A(X) { ; } };
            struct B { CONSTEXPRF B(Y) { ; } };
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
            struct A { CONSTEXPRF A(X) { ; } };
            struct B { CONSTEXPRF B(X) { ; } };
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
            struct A { CONSTEXPRF A() = default; CONSTEXPRF A(B &&) { ; } };
            struct B { CONSTEXPRF B() = default; CONSTEXPRF B(A) { ; } };
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
            struct A { CONSTEXPRF A(X, Y) { ; } };
            struct B { CONSTEXPRF B(Y, X) { ; } };
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

    CONSTEXPRF
    static
    bool
    run() noexcept
    {
        ASSERT (trivially_default_constructible());
        ASSERT (trivially_copy_move_constructible());
        ASSERT (trivially_copy_move_assignable());
        ASSERT (convertible()); // conversion
        ASSERT (constructible()); // conversion
        ASSERT (assignable()); // conversion
        ASSERT (in_place_constructible());
        ASSERT (emplace());
        return true;
    }

};

template< template< typename ... > class wrapper,
          template< typename ... > class versatile >
class check_utility
{

    template< typename ...types >
    using V = versatile< typename wrapper< types >::type... >;

    CONSTEXPRF
    static
    bool
    check_swap() noexcept
    {
        using std::swap;
        {
            struct A { int state; };
            struct B { int state; };
            using U = V< A, B >;
            U a = A{1};
            CHECK (is_active< A >(a));
            U b = B{2};
            CHECK (is_active< B >(b));
            CHECK (static_cast< A & >(a).state == 1);
            CHECK (static_cast< B & >(b).state == 2);
            swap(a, b);
            CHECK (is_active< A >(b));
            CHECK (is_active< B >(a));
            CHECK (static_cast< A & >(b).state == 1);
            CHECK (static_cast< B & >(a).state == 2);
        }
        {
            struct A { int state; };
            struct B {};
            using U = V< A, B >;
            U x = A{1};
            CHECK (is_active< A >(x));
            U y = A{2};
            CHECK (is_active< A >(y));
            CHECK (static_cast< A & >(x).state == 1);
            CHECK (static_cast< A & >(y).state == 2);
            swap(x, y);
            CHECK (is_active< A >(x));
            CHECK (is_active< A >(y));
            CHECK (static_cast< A & >(x).state == 2);
            CHECK (static_cast< A & >(y).state == 1);
        }
        return true;
    }

    CONSTEXPRF
    static
    bool
    check_compare() noexcept
    {
        {
            using U = V< int >;
            CHECK(U{1} == U{1});
            CHECK(U{1} == 1);
            CHECK(1 == U{1});
        }
        {
            using U = V< int >;
            CHECK(U{1} < U{2});
            CHECK(!(U{2} < U{1}));
            CHECK(U{1} < 2);
            CHECK(!(U{2} < 1));
            CHECK(1 < U{2});
            CHECK(!(2 < U{1}));
        }
        {
            using U = V< int >;
            CHECK(!(U{1} < U{1}));
            CHECK(!(U{1} < 1));
            CHECK(!(1 < U{1}));
        }
        return true;
    }

public :

    CONSTEXPRF
    static
    bool
    run() noexcept
    {
        ASSERT (check_swap());
        ASSERT (check_swap());
        return true;
    }

};

} // namespace test
