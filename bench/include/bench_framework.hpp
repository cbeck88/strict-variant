#pragma once

#include "bench.hpp"
#include "bench_api.hpp"

#include <chrono>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <utility>

namespace benchmark {

// Should be instantiated using a bench_task type specialized for the variant
// and the task complexity, and a VisitorApplier function object which visits
// the variant, and a seed for the rng.
template <template <class...> class var_t, uint32_t num_variants, uint32_t seq_length,
          uint32_t repeat_num, typename VisitorApplier,
          typename ClockType = std::chrono::high_resolution_clock>
void
run_benchmark(const char * variant_name, const uint32_t seed) {
  using BenchTask_t = bench_task<var_t, num_variants, seq_length>;
  BenchTask_t task{seed};

  std::fprintf(stdout, "%s:\n  num_variants = %u\n  seq_length = %u\n  repeat_num = %u\n\n",
               variant_name, num_variants, seq_length, repeat_num);

  benchmark::DoNotOptimize(task);

  auto const start = ClockType::now();

  benchmark::ClobberMemory();

  for (uint32_t count{repeat_num}; count; --count) {
    task.run(VisitorApplier{});
  }

  auto const end = ClockType::now();

  benchmark::ClobberMemory();

  unsigned long us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
  std::fprintf(stdout, "took %lu microseconds\n", us);
  std::fprintf(stdout, "average nanoseconds per visit: %f\n\n\n",
               (static_cast<double>(us) / (seq_length * repeat_num)) * 1000);
}

} // end namespace benchmark
