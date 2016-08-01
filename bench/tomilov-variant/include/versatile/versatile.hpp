#pragma once

#include <versatile/in_place.hpp>
#include <versatile/visit.hpp>

#include <type_traits>
#include <utility>
#include <typeinfo>

#include <cassert>
#include <cstddef>

namespace versatile
{

template< bool trivially_destructible, typename ...types >
class constructor_dispatcher;

template< bool trivially_destructible >
class constructor_dispatcher< trivially_destructible >
{

};

template< typename first, typename ...rest >
class constructor_dispatcher< true, first, rest... >
{

    union
    {

        first head_;
        constructor_dispatcher< true, rest... > tail_;

    };

public :

    template< typename ...arguments >
    constexpr
    constructor_dispatcher(index_t< (1 + sizeof...(rest)) >,
                           arguments &&... _arguments)
        : head_(std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments >
    constexpr
    constructor_dispatcher(arguments &&... _arguments)
        : tail_(std::forward< arguments >(_arguments)...)
    { ; }

    using this_type = unwrap_type_t< first >;

    constexpr
    operator this_type const & () const noexcept
    {
        return head_;
    }

    constexpr
    operator this_type & () noexcept
    {
        return head_;
    }

    template< typename type >
    constexpr
    operator type const & () const noexcept
    {
        return tail_;
    }

    template< typename type >
    constexpr
    operator type & () noexcept
    {
        return tail_;
    }

};

template< typename first, typename ...rest >
class constructor_dispatcher< false, first, rest... >
{

    union
    {

        first head_;
        constructor_dispatcher< false, rest... > tail_;

    };

public :

    constructor_dispatcher(constructor_dispatcher const &) = default;
    constructor_dispatcher(constructor_dispatcher &) = default;
    constructor_dispatcher(constructor_dispatcher &&) = default;

    constructor_dispatcher & operator = (constructor_dispatcher const &) = default;
    constructor_dispatcher & operator = (constructor_dispatcher &) = default;
    constructor_dispatcher & operator = (constructor_dispatcher &&) = default;

    ~constructor_dispatcher() noexcept
    { ; }

    template< typename ...arguments >
    constexpr
    constructor_dispatcher(index_t< (1 + sizeof...(rest)) >,
                           arguments &&... _arguments)
        : head_(std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments >
    constexpr
    constructor_dispatcher(arguments &&... _arguments)
        : tail_(std::forward< arguments >(_arguments)...)
    { ; }

    void
    destruct(in_place_t (&)(first)) noexcept
    {
        head_.~first();
    }

    template< typename type >
    void
    destruct(in_place_t (&)(type)) noexcept
    {
        tail_.destruct(in_place< type >);
    }

    using this_type = unwrap_type_t< first >;

    constexpr
    operator this_type const & () const noexcept
    {
        return head_;
    }

    constexpr
    operator this_type & () noexcept
    {
        return head_;
    }

    template< typename type >
    constexpr
    operator type const & () const noexcept
    {
        return tail_;
    }

    template< typename type >
    constexpr
    operator type & () noexcept
    {
        return tail_;
    }

};

template< bool trivially_destructible, typename ...types >
class destructor_dispatcher;

template< typename ...types >
class destructor_dispatcher< true, types... >
{

    std::size_t which_;
    constructor_dispatcher< true, types... > storage_;

public :

    constexpr
    std::size_t
    which() const noexcept
    {
        return which_;
    }

    template< typename index, typename ...arguments >
    constexpr
    destructor_dispatcher(index, arguments &&... _arguments)
        : which_{index::value}
        , storage_(index{}, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type >
    constexpr
    operator type const & () const noexcept
    {
        return storage_;
    }

    template< typename type >
    constexpr
    operator type & () noexcept
    {
        return storage_;
    }

};

template< typename ...types >
class destructor_dispatcher< false, types... >
{

    using storage = constructor_dispatcher< false, types... >;

    std::size_t which_;
    storage storage_;

    template< typename type >
    static
    void
    destruct(storage & _storage) noexcept
    {
        _storage.destruct(in_place< type >);
    }

    using destructor = decltype(&destructor_dispatcher::template destruct< typename identity< types... >::type >);

    static constexpr destructor destructors_[sizeof...(types)] = {destructor_dispatcher::template destruct< types >...};

public :

    constexpr
    std::size_t
    which() const noexcept
    {
        return which_;
    }

    destructor_dispatcher(destructor_dispatcher const &) = default;
    destructor_dispatcher(destructor_dispatcher &) = default;
    destructor_dispatcher(destructor_dispatcher &&) = default;

    destructor_dispatcher & operator = (destructor_dispatcher const &) = default;
    destructor_dispatcher & operator = (destructor_dispatcher &) = default;
    destructor_dispatcher & operator = (destructor_dispatcher &&) = default;

    ~destructor_dispatcher() noexcept
    {
        assert(!(sizeof...(types) < which_));
        if (0 < which_) {
            destructors_[sizeof...(types) - which_](storage_);
        }
    }

    template< typename index, typename ...arguments >
    constexpr
    destructor_dispatcher(index, arguments &&... _arguments)
        : which_{index::value}
        , storage_(index{}, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type >
    constexpr
    operator type const & () const noexcept
    {
        return storage_;
    }

    template< typename type >
    constexpr
    operator type & () noexcept
    {
        return storage_;
    }

};

template< typename ...types >
constexpr typename destructor_dispatcher< false, types... >::destructor destructor_dispatcher< false, types... >::destructors_[sizeof...(types)];

template< bool default_constructible >
struct enable_default_constructor;

template<>
struct enable_default_constructor< true >
{

    using enabler = enable_default_constructor;

    enable_default_constructor() = default;

    constexpr
    enable_default_constructor(void *)
    { ; }

};

template<>
struct enable_default_constructor< false >
{

    using enabler = enable_default_constructor;

    enable_default_constructor() = delete;

    constexpr
    enable_default_constructor(void *)
    { ; }

};

template< typename ...types >
class versatile
        : enable_default_constructor< (is_constructible_v< types > || ...) >
{

    destructor_dispatcher< (std::is_trivially_destructible_v< types > && ...), types... > storage_;

public :

    using variant_type = versatile;

    using types_t = identity< unwrap_type_t< types >... >;
    using indices_t = std::index_sequence_for< types... >;

    template< typename type >
    using index_at_t = index_at_t< unwrap_type_t< type >, unwrap_type_t< types >... >;

    template< typename ...arguments >
    using index_of_constructible_t = get_index_t< is_constructible_v< types, arguments... >... >;

    constexpr
    std::size_t
    which() const noexcept
    {
        return storage_.which();
    }

    template< typename type >
    constexpr
    bool
    active() const noexcept
    {
        return (storage_.which() == index_at_t< type >::value);
    }

    constexpr
    versatile()
        : versatile(in_place<>)
    { ; }

    template< std::size_t i, typename ...arguments >
    explicit
    constexpr
    versatile(in_place_t (&)(index_t< i >), arguments &&... _arguments)
        : versatile::enabler({})
        , storage_(index_t< i >{}, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename type,
              typename index = index_at_t< type > >
    constexpr
    versatile(type && _value)
        : versatile(in_place< index >, std::forward< type >(_value))
    { ; }

    template< typename type, typename ...arguments,
              typename index = index_at_t< type > >
    explicit
    constexpr
    versatile(in_place_t (&)(type), arguments &&... _arguments)
        : versatile(in_place< index >, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments,
              typename index = index_of_constructible_t< arguments... > >
    explicit
    constexpr
    versatile(in_place_t (&)(in_place_t), arguments &&... _arguments)
        : versatile(in_place< index >, std::forward< arguments >(_arguments)...)
    { ; }

    template< typename ...arguments,
              typename = std::enable_if_t< (1 < sizeof...(arguments)) >, // experimental
              typename index = index_of_constructible_t< arguments... > >
    explicit
    constexpr
    versatile(arguments &&... _arguments)
        : versatile(in_place< index >, std::forward< arguments >(_arguments)...)
    { ; }

    constexpr
    void
    swap(versatile & _that) noexcept(std::is_nothrow_move_assignable_v< versatile > && std::is_nothrow_move_constructible_v< versatile >)
    {
        versatile this_ = std::move(*this);
        *this = std::move(_that);
        _that = std::move(this_);
    }

    template< typename type,
              typename index = index_at_t< type > >
    constexpr
    versatile &
    operator = (type && _value) noexcept
    {
        return (*this = versatile(std::forward< type >(_value))); // http://stackoverflow.com/questions/33936295/
    }

    template< typename type,
              typename index = index_at_t< type > >
    explicit
    constexpr
    operator type const & () const
    {
        return (active< type >() ? storage_ : throw std::bad_cast{});
    }

    template< typename type,
              typename index = index_at_t< type > >
    explicit
    constexpr
    operator type & ()
    {
        return (active< type >() ? storage_ : throw std::bad_cast{});
    }

};

template<>
class versatile<>
{

};

template< typename first, typename ...rest >
struct is_visitable< versatile< first, rest... > >
      : std::true_type
{

};

template< typename first, typename ...rest >
constexpr
void
swap(versatile< first, rest... > & _lhs,
     versatile< first, rest... > & _rhs) noexcept(noexcept(_lhs.swap(_rhs)))
{
    _lhs.swap(_rhs);
}

template< std::size_t i, typename ...arguments, typename first, typename ...rest >
constexpr
void
emplace(versatile< first, rest... > & _versatile, arguments &&... _arguments)
{
    _versatile = versatile< first, rest... >(in_place< i >, std::forward< arguments >(_arguments)...);
}

template< typename type = in_place_t, typename ...arguments, typename first, typename ...rest >
constexpr
void
emplace(versatile< first, rest... > & _versatile, arguments &&... _arguments)
{
    _versatile = versatile< first, rest... >(in_place< type >, std::forward< arguments >(_arguments)...);
}

}
