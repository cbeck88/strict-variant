#pragma once

#include <versatile/recursive_wrapper.hpp>
#include <versatile/aggregate_wrapper.hpp>

namespace test
{

template< typename type >
struct aggregate
        : ::versatile::identity< ::versatile::aggregate_wrapper< type > >
{

};

template< typename type >
struct recursive_wrapper
        : ::versatile::identity< ::versatile::recursive_wrapper< type > >
{

};

}
