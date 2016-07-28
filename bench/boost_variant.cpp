#include "bench.hpp"
#include <boost/variant/variant.hpp>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <utility>

static constexpr uint32_t num_variants{NUM_VARIANTS};
static constexpr uint32_t seq_length{SEQ_LENGTH};

// Dummy visitor

struct dummy_visitor : boost::static_visitor<uint32_t> {
  template <uint32_t N>
  uint32_t operator()(const dummy<N> &) const {
    return N;
  }
};

struct visitor_applier {
  template <typename T>
  uint32_t operator()(T && t) const {
    return boost::apply_visitor(dummy_visitor{}, std::forward<T>(t));
  }
};

int
main() {
  using bench_task_t = bench_task<boost::variant, num_variants, seq_length>;
  std::unique_ptr<bench_task_t> task{new bench_task_t()};

  std::fprintf(stdout, "boost::variant:\n  num_variants = %u\n  seq_length = %u\n\n", num_variants,
               seq_length);

  auto const start = std::chrono::high_resolution_clock::now();

  uint32_t result = task->run(visitor_applier{});

  auto const end = std::chrono::high_resolution_clock::now();

  unsigned long us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  std::fprintf(stdout, "took %lu microseconds\n", us);
  std::fprintf(stdout, "average nanoseconds per visit: %f\n\n\n",
               (static_cast<double>(us) / seq_length) * 1000);

  return result != 0;
}
