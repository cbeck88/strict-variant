#pragma once

#include <versatile/type_traits.hpp>

#include <type_traits>
#include <utility>
#include <memory>

namespace versatile
{

template< typename this_type >
class recursive_wrapper
{

    std::unique_ptr< this_type > storage_;

public :

    template< typename ...arguments >
    recursive_wrapper(arguments &&... _arguments)
        : storage_(std::make_unique< this_type >(std::forward< arguments >(_arguments)...))
    { ; }

    template< typename type >
    operator type & () noexcept
    {
        return static_cast< type & >(*storage_);
    }

    template< typename type >
    operator type const & () const noexcept
    {
        return static_cast< type const & >(*storage_);
    }

    void
    swap(recursive_wrapper & _other) noexcept
    {
        storage_.swap(_other.storage_);
    }

};

template< typename type >
struct unwrap_type< recursive_wrapper< type > >
        : unwrap_type< type >
{

};

template< typename type >
struct unref_type< recursive_wrapper< type > >
        : unref_type< type >
{

};

}
