#pragma once

#include <versatile/visit.hpp>

#include <ostream>
#include <istream>

namespace versatile
{

template< typename visitable >
std::enable_if_t< is_visitable_v< visitable >, std::istream & >
operator >> (std::istream & _in, visitable & _visitable)
{
    visit([&] (auto & _value) { _in >> _value; }, _visitable);
    return _in;
}

template< typename visitable >
std::enable_if_t< is_visitable_v< visitable >, std::ostream & >
operator << (std::ostream & _out, visitable const & _visitable)
{
    visit([&] (auto const & _value) { _out << _value; }, _visitable);
    return _out;
}

}
