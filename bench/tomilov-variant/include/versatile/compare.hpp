#pragma once

#include "variant.hpp"

#include <type_traits>
#include <typeinfo>

namespace versatile
{

struct equal_to
{

    template< typename type >
    constexpr
    bool
    operator () (type const & _lhs, type const & _rhs) const
    {
        return static_cast< bool >(_lhs == _rhs);
    }

    template< typename lhs, typename rhs >
    bool
    operator () (lhs const &, rhs const &) const
    {
        throw std::bad_cast{};
    }

};

template< typename lhs, typename rhs >
constexpr
std::enable_if_t< (is_visitable_v< lhs > || is_visitable_v< rhs >), bool >
operator == (lhs const & _lhs, rhs const & _rhs)
{
    return multivisit(equal_to{}, _lhs, _rhs);
}

struct less
{

    template< typename type >
    constexpr
    bool
    operator () (type const & _lhs, type const & _rhs) const
    {
        return static_cast< bool >(_lhs < _rhs);
    }

    template< typename lhs, typename rhs >
    bool
    operator () (lhs const &, rhs const &) const
    {
        throw std::bad_cast{};
    }

};

template< typename lhs, typename rhs >
constexpr
std::enable_if_t< (is_visitable_v< lhs > || is_visitable_v< rhs >), bool >
operator < (lhs const & _lhs, rhs const & _rhs)
{
    return multivisit(less{}, _lhs, _rhs);
}

}
