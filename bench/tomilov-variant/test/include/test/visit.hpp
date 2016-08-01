#pragma once

#include "test/prologue.hpp"

#include <versatile/visit.hpp>
#include <versatile/utility.hpp>

#include <type_traits>
#include <utility>

namespace test
{

template< typename F, std::size_t ...indices >
struct enumerator
{

    static constexpr std::size_t size_ = sizeof...(indices);
    static constexpr std::size_t count_ = (indices * ...);

    template< typename O, typename D >
    struct decomposer;

    template< std::size_t ...O, std::size_t ...D >
    struct decomposer< std::index_sequence< O... >, std::index_sequence< D... > >
    {

        F & f;

        static constexpr std::size_t indices_[size_] = {indices...};

        static
        constexpr
        std::size_t
        order(std::size_t const i) noexcept
        {
            std::size_t o = 1;
            for (std::size_t n = i + 1; n < size_; ++n) {
                o *= indices_[n];
            }
            return o;
        }

        static constexpr std::size_t orders_[size_] = {order(O)...};

        static
        constexpr
        std::size_t
        digit(std::size_t d, std::size_t const o) noexcept
        {
            for (std::size_t n = 0; n < o; ++n) {
                d = d % orders_[n];
            }
            return d / orders_[o];
        }

        template< std::size_t d >
        using index_sequence = std::index_sequence< digit(d, O)... >;

        CONSTEXPRF
        bool
        operator () () const noexcept
        {
            return (f(index_sequence< D >{}) && ...);
        }

    };

    decomposer< std::make_index_sequence< size_ >, std::make_index_sequence< count_ > > const decomposer_;

    CONSTEXPRF
    bool
    operator () () const noexcept
    {
        return decomposer_();
    }

};

template< std::size_t ...indices, typename F >
CONSTEXPRF
enumerator< F, indices... >
make_enumerator(F & f) noexcept
{
    SA(0 < sizeof...(indices));
    SA(((0 < indices) && ...));
    return {{f}};
}

using ::versatile::type_qualifier;

template< std::size_t M >
struct pair
{

    type_qualifier qual_ids[1 + M];
    std::size_t type_ids[1 + M];

    CONSTEXPRF
    bool
    operator == (pair const & _rhs) const noexcept
    {
        for (std::size_t i = 0; i <= M; ++i) {
            if (qual_ids[i] != _rhs.qual_ids[i]) {
                return false;
            }
            if (type_ids[i] != _rhs.type_ids[i]) {
                return false;
            }
        }
        return true;
    }

    CONSTEXPRF
    bool
    operator != (pair const & _rhs) const noexcept
    {
        return !operator == (_rhs);
    }

    CONSTEXPRF
    std::size_t
    size() const noexcept
    {
        return (1 + M);
    }

};

using ::versatile::type_qualifier_of;

template< std::size_t M, type_qualifier type_qual = type_qualifier::value >
struct multivisitor
{

    using result_type = pair< M >;

    result_type & result_;

    using return_type = ::versatile::add_type_qualifier_t< type_qual, result_type >;

    static constexpr type_qualifier type_qual_ = type_qual;

    CONSTEXPRF
    std::size_t
    which() const noexcept
    {
        return M;
    }

    template< typename ...types >
    CONSTEXPRF
    return_type
    operator () (types &&... _values) & noexcept
    {
        //ASSERT (M == sizeof...(types));
        //ASSERT (!(is_visitable_v< types > || ...));
        result_ = {{type_qualifier_of< multivisitor & >, type_qualifier_of< types && >...}, {which(), _values.get_state()...}};
        return static_cast< return_type >(result_);
    }

    template< typename ...types >
    CONSTEXPRF
    return_type
    operator () (types &&... _values) const & noexcept
    {
        result_ = {{type_qualifier_of< multivisitor const & >, type_qualifier_of< types && >...}, {which(), _values.get_state()...}};
        return static_cast< return_type >(result_);
    }

    template< typename ...types >
    CONSTEXPRF
    return_type
    operator () (types &&... _values) && noexcept
    {
        result_ = {{type_qualifier_of< multivisitor && >, type_qualifier_of< types && >...}, {which(), _values.get_state()...}};
        return static_cast< return_type >(result_);
    }

    template< typename ...types >
    CONSTEXPRF
    return_type
    operator () (types &&... _values) const && noexcept
    {
        result_ = {{type_qualifier_of< multivisitor const && >, type_qualifier_of< types && >...}, {which(), _values.get_state()...}};
        return static_cast< return_type >(result_);
    }

};

template< typename array_type >
struct subscripter
{

    array_type & array_;

    constexpr
    array_type &
    operator () () const noexcept
    {
        return array_;
    }

    template< typename first, typename ...rest >
    constexpr
    decltype(auto)
    operator () (first const & _first, rest const &... _rest) const noexcept
    {
        return operator () (_rest...)[_first];
    }

};

template< typename array_type, typename ...indices >
constexpr
decltype(auto)
subscript(array_type & _array, indices const &... _indices) noexcept
{
    return subscripter< array_type >{_array}(_indices...);
}

constexpr auto type_qual_begin = static_cast< std::size_t >(type_qualifier_of< void * & >);
constexpr auto type_qual_end = static_cast< std::size_t >(type_qualifier_of< void * volatile & >);

template< typename multivisitor, typename variants, typename result_type >
struct fusor
{

    static constexpr std::size_t M = std::extent< variants >::value;

    template< typename = std::make_index_sequence< M > >
    struct fuse;

    template< std::size_t ...i >
    struct fuse< std::index_sequence< i... > >
    {

        multivisitor multivisitor_;
        variants variants_;
        std::size_t counter_;
        result_type result_;

        template< std::size_t m, std::size_t ...v >
        CONSTEXPRF
        bool
        operator () (std::index_sequence< m, v... >) noexcept
        {
            SA(M == sizeof...(v));
            constexpr type_qualifier type_qual_m = static_cast< type_qualifier >(type_qual_begin + m);
            constexpr type_qualifier type_quals_v[sizeof...(v)] = {static_cast< type_qualifier >(type_qual_begin + v)...};
            pair< M > const rhs = {{type_qual_m, type_quals_v[i]...}, {M, variants_[i].which()...}};
            using ::versatile::forward_as;
            using ::versatile::multivisit;
            decltype(auto) lhs = multivisit(forward_as< type_qual_m >(multivisitor_),
                                            forward_as< type_quals_v[i] >(variants_[i])...);
            CHECK (type_qualifier_of< decltype(lhs) > == multivisitor_.type_qual_);
            CHECK (M + 1 == lhs.size());
            if (!(lhs == rhs)) {
                return false;
            }
            bool & r = subscript(result_, m, v..., (variants_[i].which() - 1)...);
            if (r) {
                return false;
            }
            r = true;
            ++counter_;
            return true;
        }

    };

    fuse<> fuse_;

    constexpr
    auto &
    operator [] (std::size_t const i) noexcept
    {
        return fuse_.variants_[i];
    }

};

template< typename value_type, std::size_t ...extents >
struct multiarray;

template< typename array_type >
struct multiarray< array_type >
{

    using type = array_type;

};

template< typename type, std::size_t first, std::size_t ...rest >
struct multiarray< type, first, rest... >
        : multiarray< type[first], rest... >
{

    using value_type = type;

};

template< typename value_type, std::size_t ...extents >
using multiarray_t = typename multiarray< value_type, extents... >::type;

constexpr std::size_t qual_count_ = (type_qual_end - type_qual_begin);

// variant - variant
// type - type generator
// variant - variant template
// wrapper - wrapper for alternative (bounded) types
// M - multivisitor arity, N - number of alternative (bounded) types
template< template< std::size_t I > class type,
          template< typename ...types > class variant,
          template< typename ...types > class wrapper,
          std::size_t M = 2, std::size_t N = M >
class perferct_forwarding
{

    template< type_qualifier type_qual,
              std::size_t ...i, std::size_t ...j >
    CONSTEXPRF
    static
    bool
    run(std::index_sequence< i... >, std::index_sequence< j... >) noexcept
    {
        using multivisitor_type = multivisitor< M, type_qual >;
        typename multivisitor_type::result_type result_{};
        using variant_type = variant< typename wrapper< type< N - j > >::type... >;
        using result_type = multiarray_t< bool, qual_count_, (static_cast< void >(i), qual_count_)..., (static_cast< void >(i), N)... >;
        fusor< multivisitor_type, variant_type [M], result_type > fusor_{{{result_}, {}, 0, {}}};
        auto const enumerator_ = make_enumerator< qual_count_, (static_cast< void >(i), qual_count_)... >(fusor_.fuse_);
        variant_type const variants_[N] = {type< N - j >{N - j}...};
        CHECK (((variants_[j].which() == (N - j)) && ...));
        std::size_t indices[M] = {};
        for (;;) {
            ((fusor_[i] = variants_[indices[i]]), ...);
            if (!enumerator_()) {
                return false;
            }
            std::size_t m = 0;
            for (;;) {
                std::size_t & n = indices[m];
                if (++n != N) {
                    break;
                }
                n = 0;
                if (++m == M) {
                    break;
                }
            }
            if (m == M) {
                break;
            }
        }
        constexpr std::size_t count_ = ((static_cast< void >(i), (N * qual_count_)) * ...) * qual_count_; // N ^ M * qual_count_ ^ (M + 1)
        CHECK (fusor_.fuse_.counter_ == count_);
        SA(sizeof(result_type) == count_ * sizeof(bool)); // sizeof(bool) is implementation-defined
        return true;
    }

public :

    CONSTEXPRF
    static
    bool
    run() noexcept
    {
        constexpr auto i = std::make_index_sequence< M >{};
        constexpr auto j = std::make_index_sequence< N >{};
        CHECK (run< type_qualifier::value       >(i, j));
        CHECK (run< type_qualifier::const_value >(i, j));
        CHECK (run< type_qualifier::lref        >(i, j));
        CHECK (run< type_qualifier::rref        >(i, j));
        CHECK (run< type_qualifier::const_lref  >(i, j));
        CHECK (run< type_qualifier::const_rref  >(i, j));
        return true;
    }

};

} // namespace test
