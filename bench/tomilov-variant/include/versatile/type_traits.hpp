#pragma once

#include <type_traits>

namespace versatile
{

template< typename first, typename ...rest >
struct identity
{

    using type = first;

};

template< std::size_t i >
using index_t = std::integral_constant< std::size_t, i >;

enum class type_qualifier
{
    value,
    const_value,
    volatile_value,
    volatile_const_value,
    lref,
    rref,
    const_lref,
    const_rref,
    volatile_lref,
    volatile_rref,
    volatile_const_lref,
    volatile_const_rref,

    count_
};

template< type_qualifier type_qual, typename type > struct add_type_qualifier;
template< typename to > struct add_type_qualifier< type_qualifier::value                , to > { using type =          to         ; };
template< typename to > struct add_type_qualifier< type_qualifier::const_value          , to > { using type =          to const   ; };
template< typename to > struct add_type_qualifier< type_qualifier::volatile_value       , to > { using type = volatile to         ; };
template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_value , to > { using type = volatile to const   ; };
template< typename to > struct add_type_qualifier< type_qualifier::lref                 , to > { using type =          to       & ; };
template< typename to > struct add_type_qualifier< type_qualifier::rref                 , to > { using type =          to       &&; };
template< typename to > struct add_type_qualifier< type_qualifier::const_lref           , to > { using type =          to const & ; };
template< typename to > struct add_type_qualifier< type_qualifier::const_rref           , to > { using type =          to const &&; };
template< typename to > struct add_type_qualifier< type_qualifier::volatile_lref        , to > { using type = volatile to       & ; };
template< typename to > struct add_type_qualifier< type_qualifier::volatile_rref        , to > { using type = volatile to       &&; };
template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_lref  , to > { using type = volatile to const & ; };
template< typename to > struct add_type_qualifier< type_qualifier::volatile_const_rref  , to > { using type = volatile to const &&; };

template< type_qualifier type_qual, typename to >
using add_type_qualifier_t = typename add_type_qualifier< type_qual, to >::type;

template< typename from > constexpr type_qualifier type_qualifier_of                           = type_qualifier::value                ;
template< typename from > constexpr type_qualifier type_qualifier_of<          from const    > = type_qualifier::const_value          ;
template< typename from > constexpr type_qualifier type_qualifier_of< volatile from          > = type_qualifier::volatile_value       ;
template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const    > = type_qualifier::volatile_const_value ;
template< typename from > constexpr type_qualifier type_qualifier_of<          from       &  > = type_qualifier::lref                 ;
template< typename from > constexpr type_qualifier type_qualifier_of<          from       && > = type_qualifier::rref                 ;
template< typename from > constexpr type_qualifier type_qualifier_of<          from const &  > = type_qualifier::const_lref           ;
template< typename from > constexpr type_qualifier type_qualifier_of<          from const && > = type_qualifier::const_rref           ;
template< typename from > constexpr type_qualifier type_qualifier_of< volatile from       &  > = type_qualifier::volatile_lref        ;
template< typename from > constexpr type_qualifier type_qualifier_of< volatile from       && > = type_qualifier::volatile_rref        ;
template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const &  > = type_qualifier::volatile_const_lref  ;
template< typename from > constexpr type_qualifier type_qualifier_of< volatile from const && > = type_qualifier::volatile_const_rref  ;

template< typename from, typename to >
using copy_cv_reference_t = add_type_qualifier_t< type_qualifier_of< from >, to >;

template< typename type, typename ...types >
struct index_at // characteristic is 1-based right-to-left index of leftmost matched type if any
{

};

template< typename type, typename ...rest >
struct index_at< type, type, rest... >
        : index_t< (1 + sizeof...(rest)) >
{

};

template< typename type, typename first, typename ...rest >
struct index_at< type, first, rest... >
        : index_at< type, rest... >
{

};

template< typename type, typename ...types >
using index_at_t = typename index_at< type, types... >::type;

template< std::size_t i, typename ...types >
struct at_index // i treated as 1-based right-to-left index
{

};

template< typename first, typename ...rest >
struct at_index< (1 + sizeof...(rest)), first, rest... >
    : identity< first >
{

};

template< std::size_t i, typename first, typename ...rest >
struct at_index< i, first, rest... >
        : at_index< i, rest... >
{

};

template< std::size_t i, typename ...types >
using at_index_t = typename at_index< i, types... >::type;

template< bool ...values >
struct get_index; // characteristic is 1-based right-to-left index of leftmost true if any

template<>
struct get_index<>
{

};

template< bool ...rest >
struct get_index< true, rest... >
        : index_t< (1 + sizeof...(rest)) >
{

};

template< bool ...rest >
struct get_index< false, rest... >
        : get_index< rest... >
{

};

template< bool ...values >
using get_index_t = typename get_index< values... >::type;

template< typename type >
struct is_visitable
        : std::false_type
{

};

template< typename type >
constexpr bool is_visitable_v = is_visitable< type >::value;

template< typename type >
struct unref_type
        : identity< type >
{

};

template< typename type >
using unref_type_t = typename unref_type< type >::type;

template< typename type >
struct unwrap_type
        : identity< type >
{

};

template< typename type >
using unwrap_type_t = typename unwrap_type< std::decay_t< type > >::type;

template< typename type, typename ...arguments >
struct is_constructible // may be specialized for incomplete types wrapped in recursive_wrapper
    : std::is_constructible< type, arguments... >
{

};

template< typename type, typename ...arguments >
constexpr bool is_constructible_v = is_constructible< type, arguments... >::value;

}
