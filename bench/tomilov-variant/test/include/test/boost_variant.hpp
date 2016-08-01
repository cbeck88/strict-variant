#pragma once

#include <boost/variant.hpp>

#include <versatile/type_traits.hpp>

#include "test/prologue.hpp"

#include <type_traits>
#include <utility>
#include <typeinfo>

namespace test_boost_variant
{

using ::boost::get;
using ::versatile::index_at_t;
using ::versatile::get_index;
using ::versatile::unwrap_type_t;

template< typename ...types >
class boost_variant_c // composition
{

    using variant = ::boost::variant< types... >;

    variant storage_;

public :

    template< typename type >
    using index_at_t = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >... >;

    template< typename ...arguments >
    using index_of_constructible = get_index< std::is_constructible_v< types, arguments... >... >;

    boost_variant_c() = default;

    boost_variant_c(boost_variant_c const &) = default;
    boost_variant_c(boost_variant_c &) = default;
    boost_variant_c(boost_variant_c &&) = default;

    boost_variant_c & operator = (boost_variant_c const &) = default;
    boost_variant_c & operator = (boost_variant_c &) = default;
    boost_variant_c & operator = (boost_variant_c &&) = default;

    template< typename type, typename = index_at_t< type > >
    boost_variant_c(type && _value)
        : storage_(std::forward< type >(_value))
    { ; }

    template< typename type, typename = index_at_t< type > >
    boost_variant_c &
    operator = (type && _value)
    {
        storage_ = std::forward< type >(_value);
        return *this;
    }

    std::size_t
    which() const
    {
        return sizeof...(types) - static_cast< std::size_t >(storage_.which());
    }

    template< typename type >
    bool
    active() const noexcept
    {
        return (index_at_t< type >::value == which());
    }

    template< typename type, typename = index_at_t< type > >
    explicit
    operator type const & () const
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return get< type >(storage_);
    }

    template< typename type, typename = index_at_t< type > >
    explicit
    operator type & ()
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return get< type >(storage_);
    }

};

template< typename type, typename ...types >
CONSTEXPRF
bool
is_active(boost_variant_c< types... > const & v) noexcept
{
    return v.template active< type >();
}

template< typename ...types >
class boost_variant_i // inheritance
        : private ::boost::variant< types... >
{

    using base = ::boost::variant< types... >;

public :

    template< typename type >
    using index_at_t = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >... >;

    template< typename ...arguments >
    using index_of_constructible = get_index< std::is_constructible_v< types, arguments... >... >;

    //using base::base; // seems there is wrong design of boost::variant constructor
    //using base::operator =;

    boost_variant_i() = default;

    boost_variant_i(boost_variant_i const &) = default;
    boost_variant_i(boost_variant_i &) = default;
    boost_variant_i(boost_variant_i &&) = default;

    boost_variant_i & operator = (boost_variant_i const &) = default;
    boost_variant_i & operator = (boost_variant_i &) = default;
    boost_variant_i & operator = (boost_variant_i &&) = default;

    template< typename type, typename = index_at_t< type > >
    boost_variant_i(type && _value)
        : base(std::forward< type >(_value))
    { ; }

    template< typename type, typename = index_at_t< type > >
    boost_variant_i &
    operator = (type && _value)
    {
        base::operator = (std::forward< type >(_value));
        return *this;
    }

    std::size_t
    which() const
    {
        return sizeof...(types) - static_cast< std::size_t >(base::which());
    }

    template< typename type >
    bool
    active() const noexcept
    {
        return (index_at_t< type >::value == which());
    }

    template< typename type, typename = index_at_t< type > >
    explicit
    operator type const & () const
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return get< type >(static_cast< base const & >(*this));
    }

    template< typename type, typename = index_at_t< type > >
    explicit
    operator type & ()
    {
        if (!active< type >()) {
            throw std::bad_cast{};
        }
        return get< type >(static_cast< base & >(*this));
    }

};

template< typename type, typename ...types >
CONSTEXPRF
bool
is_active(boost_variant_i< types... > const & v) noexcept
{
    return v.template active< type >();
}

template< typename type >
struct boost_recursive_wrapper
        : ::versatile::identity< ::boost::recursive_wrapper< type > >
{

};

} // namespace test_boost_variant

namespace versatile
{

template< typename type >
struct unwrap_type< ::boost::recursive_wrapper< type > >
        : unwrap_type< type >
{

};

template< typename first, typename ...rest >
struct is_visitable< ::test_boost_variant::boost_variant_i< first, rest... > >
        : std::true_type
{

};

template< typename first, typename ...rest >
struct is_visitable< ::test_boost_variant::boost_variant_c< first, rest... > >
        : std::true_type
{

};

} // namespace versatile
