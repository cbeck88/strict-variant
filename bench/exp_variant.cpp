#include "bench.hpp"
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <experimental/variant.hpp>
#include <memory>
#include <utility>

static constexpr uint32_t num_variants{NUM_VARIANTS};
static constexpr uint32_t seq_length{SEQ_LENGTH};
static constexpr uint32_t repeat_num{REPEAT_NUM};

// Dummy visitor

struct dummy_visitor {
  template <uint32_t N>
  uint32_t operator()(const dummy<N> &) const {
    return N;
  }
};

struct visitor_applier {
  template <typename T>
  uint32_t operator()(T && t) const {
    return std::experimental::visit(dummy_visitor{}, std::forward<T>(t));
  }
};

int
main() {
  using bench_task_t = bench_task<std::experimental::variant, num_variants, seq_length>;
  std::unique_ptr<bench_task_t> task{new bench_task_t(RNG_SEED)};

  std::fprintf(stdout, "experimental::variant:\n  num_variants = %u\n  seq_length = %u\n  repeat_num = %u\n\n", num_variants,
               seq_length, repeat_num);

  auto const start = std::chrono::high_resolution_clock::now();

  uint32_t result = 0;
  uint32_t count = repeat_num;
  while (count--) {
   DoNotOptimize(result);
   result += task->run(visitor_applier{});
   result *= 3;
   // DoNotOptimize(task->run(visitor_applier{}));
   ClobberMemory();
  }

  auto const end = std::chrono::high_resolution_clock::now();

  unsigned long us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  std::fprintf(stdout, "took %lu microseconds\n", us);
  std::fprintf(stdout, "average nanoseconds per visit: %f\n\n\n",
               (static_cast<double>(us) / (seq_length * repeat_num)) * 1000);

  return result != 0;
}
