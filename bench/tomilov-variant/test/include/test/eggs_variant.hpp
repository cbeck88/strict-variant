#pragma once

#include <eggs/variant.hpp>

#include <versatile/in_place.hpp>

#include "test/prologue.hpp"

namespace test_eggs_variant
{

using ::versatile::index_t;
using ::versatile::index_at_t;
using ::versatile::get_index_t;
using ::versatile::unwrap_type_t;
using ::versatile::in_place;
using ::versatile::in_place_t;

template< typename ...types >
class eggs_variant_c // composition
{

    ::eggs::variant< types... > storage_;

public :

    template< typename type >
    using index_at_t = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >... >;

    template< typename ...arguments >
    using index_of_constructible_t = get_index_t< std::is_constructible_v< types, arguments... >... >;

    eggs_variant_c(eggs_variant_c const &) = default;
    eggs_variant_c(eggs_variant_c &) = default;
    eggs_variant_c(eggs_variant_c &&) = default;

    eggs_variant_c & operator = (eggs_variant_c const &) = default;
    eggs_variant_c & operator = (eggs_variant_c &) = default;
    eggs_variant_c & operator = (eggs_variant_c &&) = default;

    constexpr
    eggs_variant_c()
        : eggs_variant_c(in_place<>)
    { ; }

    template< std::size_t i, typename ...arguments >
    explicit
    constexpr
    eggs_variant_c(in_place_t (&)(index_t< i >), arguments &&... _arguments)
        : storage_(::eggs::variants::in_place< (sizeof...(types) - i) >, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type, typename index = index_at_t< type > >
    constexpr
    eggs_variant_c(type && _value)
        : eggs_variant_c(in_place< index >, std::forward< type >(_value))
    { ; }

    template< typename type, typename index = index_at_t< type >, typename ...arguments >
    explicit
    constexpr
    eggs_variant_c(in_place_t (&)(type), arguments &&... _arguments)
        : eggs_variant_c(in_place< index >, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments, typename index = index_of_constructible_t< arguments... > >
    explicit
    constexpr
    eggs_variant_c(in_place_t (&)(in_place_t), arguments &&... _arguments)
        : eggs_variant_c(in_place< index >, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type, typename = index_at_t< type > >
    constexpr
    eggs_variant_c &
    operator = (type && _value)
    {
        storage_ = std::forward< type >(_value);
        return *this;
    }

    constexpr
    std::size_t
    which() const
    {
        if (!storage_) {
            return 0;
        }
        return sizeof...(types) - storage_.which();
    }

    template< typename type >
    constexpr
    bool
    active() const noexcept
    {
        return (index_at_t< type >::value == which());
    }

    template< typename type, typename = index_at_t< type > >
    explicit
    constexpr
    operator type const & () const
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return *storage_.template target< type >();
    }

    template< typename type, typename = index_at_t< type > >
    explicit
    constexpr
    operator type & ()
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return *storage_.template target< type >();
    }

};

} // namespace test

namespace versatile
{

template< typename first, typename ...rest >
struct is_visitable< ::test_eggs_variant::eggs_variant_c< first, rest... > >
        : std::true_type
{

};

} // namespace versatile
