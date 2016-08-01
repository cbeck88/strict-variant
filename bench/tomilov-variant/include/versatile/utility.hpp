#pragma once

#include <versatile/in_place.hpp>

#include <type_traits>
#include <utility>

namespace versatile
{

template< type_qualifier type_qual, typename type,
          typename result_type = add_type_qualifier_t< type_qual, std::remove_reference_t< type > > >
constexpr
result_type // preserve const
forward_as(type && _value) noexcept(noexcept(static_cast< result_type >(_value)))
{
    return static_cast< result_type >(_value);
}

template< typename pattern, typename type,
          typename result_type = copy_cv_reference_t< pattern, std::remove_reference_t< type > > >
constexpr
result_type // preserve const
forward_as(type && _value) noexcept(noexcept(static_cast< result_type >(_value)))
{
    return static_cast< result_type >(_value);
}

template< typename type, typename visitable >
constexpr
bool
is_active(visitable const & _visitable) noexcept
{
    return _visitable.template active< type >();
}

template< typename visitable, typename ...arguments >
constexpr
visitable
make_variant(arguments &&... _arguments) noexcept(noexcept(::new (std::declval< void * >()) visitable{in_place<>, std::declval< arguments >()...}))
{
    return visitable{in_place<>, std::forward< arguments >(_arguments)...};
}

namespace details
{

template< typename visitor, typename ...visitors >
struct composite_visitor
        : unwrap_type_t< visitor >
        , composite_visitor< visitors... >
{

    using head = unwrap_type_t< visitor >;
    using tail = composite_visitor< visitors... >;

    using head::operator ();
    using tail::operator ();

    constexpr
    composite_visitor(visitor & _visitor, visitors &... _visitors) noexcept(noexcept(::new (std::declval< void * >()) head(std::declval< visitor >())) && noexcept(tail{_visitors...}))
        : head(std::forward< visitor >(_visitor))
        , tail{_visitors...}
    { ; }

};

template< typename visitor >
struct composite_visitor< visitor >
        : unwrap_type_t< visitor >
{

    using base = unwrap_type_t< visitor >;

    using base::operator ();

    constexpr
    composite_visitor(visitor & _visitor) noexcept(noexcept(::new (std::declval< void * >()) base(std::declval< visitor >())))
        : base(std::forward< visitor >(_visitor))
    { ; }

};

}

template< typename visitor, typename ...visitors,
          typename result_type = details::composite_visitor< visitor, visitors... > >
constexpr
result_type
compose_visitors(visitor && _visitor, visitors &&... _visitors) noexcept(noexcept(::new (std::declval< void * >()) result_type{_visitor, _visitors...}))
{
    return {_visitor, _visitors...};
}

}
