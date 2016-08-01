#pragma once

#include <versatile/type_traits.hpp>

#include <utility>

namespace versatile
{

template< typename type >
struct aggregate_wrapper
    : type
{

    aggregate_wrapper() = default;

    using type::operator =;

    template< typename ...arguments,
              bool is_noexcept = noexcept(::new (std::declval< void * >()) type{std::declval< arguments >()...}) >
    constexpr
    aggregate_wrapper(arguments &&... _arguments) noexcept(is_noexcept)
        : type{std::forward< arguments >(_arguments)...}
    { ; }

};

template< typename type >
struct unwrap_type< aggregate_wrapper< type > >
        : unwrap_type< type >
{

};

}
