#pragma once

#include "test/prologue.hpp"

#include <versatile/type_traits.hpp>

#include <type_traits>

namespace test_traits
{

struct A {};
struct B {};

using ::versatile::copy_cv_reference_t;

SA(std::is_same_v< copy_cv_reference_t<          A         , B >,          B          >);
SA(std::is_same_v< copy_cv_reference_t<          A const   , B >,          B const    >);
SA(std::is_same_v< copy_cv_reference_t< volatile A         , B >, volatile B          >);
SA(std::is_same_v< copy_cv_reference_t< volatile A const   , B >, volatile B const    >);
SA(std::is_same_v< copy_cv_reference_t<          A        &, B >,          B        & >);
SA(std::is_same_v< copy_cv_reference_t<          A const  &, B >,          B const  & >);
SA(std::is_same_v< copy_cv_reference_t< volatile A        &, B >, volatile B        & >);
SA(std::is_same_v< copy_cv_reference_t< volatile A const  &, B >, volatile B const  & >);
SA(std::is_same_v< copy_cv_reference_t<          A       &&, B >,          B       && >);
SA(std::is_same_v< copy_cv_reference_t<          A const &&, B >,          B const && >);
SA(std::is_same_v< copy_cv_reference_t< volatile A       &&, B >, volatile B       && >);
SA(std::is_same_v< copy_cv_reference_t< volatile A const &&, B >, volatile B const && >);

}
