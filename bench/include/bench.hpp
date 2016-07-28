#pragma once

#include <array>
#include <cstddef>
#include <random>

/***
 * To make generic tests of visitation speed.
 */

// Dummy type
template <uint32_t N>
struct dummy {};

// uint32_tlist
template <uint32_t... is>
struct ilist {
  static constexpr uint32_t size = sizeof...(is);
};

// Append
template <typename l, uint32_t i>
struct Append;

template <uint32_t... is, uint32_t i>
struct Append<ilist<is...>, i> {
  using type = ilist<is..., i>;
};

template <typename l, uint32_t i>
using Append_t = typename Append<l, i>::type;

// Count
template <uint32_t i>
struct Count {
  using type = Append_t<typename Count<i - 1>::type, i - 1>;
};

template <>
struct Count<0> {
  using type = ilist<>;
};

template <uint32_t i>
using Count_t = typename Count<i>::type;

// Dummy variant type maker
template <template <class...> class variant_template, typename index_list>
struct dummy_variant;

template <template <class...> class variant_template, uint32_t... is>
struct dummy_variant<variant_template, ilist<is...>> {
  using type = variant_template<dummy<is>...>;
};

template <template <class...> class variant_template, uint32_t n>
using dummy_variant_t = typename dummy_variant<variant_template, Count_t<n>>::type;

// Assign a variant to the type represented by an uint32_t

template <typename T, typename S>
struct assignment_helper;

template <typename T, uint32_t... is>
struct assignment_helper<T, ilist<is...>> {
  using ftype = void (*)(T &);

  template <typename U>
  static void assign(T & t) {
    t = U{};
  }

  static constexpr std::array<ftype, sizeof...(is)> funcs() { return {{(&assign<dummy<is>>)...}}; }
};

template <template <class...> class variant_template, uint32_t N>
void
set_type(dummy_variant_t<variant_template, N> & v, uint32_t idx) {
  static constexpr auto funcs =
    assignment_helper<dummy_variant_t<variant_template, N>, Count_t<N>>::funcs();
  funcs[idx % N](v);
}

// Bench task, sets up a sequence of variant instances, and runs a task.

template <template <class...> class variant_template, uint32_t num_variants, uint32_t seq_length>
struct bench_task {
  using var_t = dummy_variant_t<variant_template, num_variants>;

  std::array<var_t, seq_length> sequence_;

  bench_task() {
    std::mt19937 rng{42};
    for (var_t & v : sequence_) {
      uint32_t x = static_cast<uint32_t>(rng());
      set_type<variant_template, num_variants>(v, x);
    }
  }

  template <typename AV>
  uint32_t run(AV && apply_visitor) const {
    uint32_t result = 0;
    for (const auto & v : sequence_) {
      static_cast<volatile void>(result ^= std::forward<AV>(apply_visitor)(v));
    }
    return result;
  }
};
