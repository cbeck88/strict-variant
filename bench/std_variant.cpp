#include "bench_api.hpp"
#include "bench_framework.hpp"

#include <cassert>
#include <chrono>
#include <time.h> // clock_gettime, CLOCK_MONOTONIC and CLOCK_REALTIME
#include <variant>

static constexpr uint32_t num_variants{NUM_VARIANTS};
static constexpr uint32_t seq_length{SEQ_LENGTH};
static constexpr uint32_t repeat_num{REPEAT_NUM};
static constexpr uint32_t rng_seed{RNG_SEED};

// Dummy visitor

struct dummy_visitor {
  template <uint32_t N>
  uint32_t operator()(const dummy<N> &) const {
    uint32_t result{N};
    // This makes the return value of the visitor opaque to the optimizer
    benchmark::DoNotOptimize(result);
    benchmark::ClobberMemory();
    return result;
  }
};

struct visitor_applier {
  template <typename T>
  uint32_t operator()(T && t) const {
    return std::visit(dummy_visitor{}, std::forward<T>(t));
  }
};

// Make a custom clock so that we don't have to link with the standard library.
// This is easier when using custom standard library instances.
// This is taken from implementation details of libcxx at chrono.cpp

class custom_clock {
public:
  typedef std::chrono::nanoseconds duration;
  typedef long long rep;
  typedef std::nano period;
  typedef std::chrono::time_point<custom_clock, duration> time_point;
  static constexpr bool is_steady = true;

  static time_point now() noexcept {
#ifdef CLOCK_MONOTONIC
    struct timespec tp;
    if (0 != clock_gettime(CLOCK_MONOTONIC, &tp))
      assert(false && "clock_gettime(CLOCK_MONOTONIC) failed");
    return time_point(std::chrono::seconds(tp.tv_sec) + std::chrono::nanoseconds(tp.tv_nsec));
#else
#error                                                                                             \
  "monotonic clock is needed for this implementation of steady_clock, go check chrono.cpp in libc++ for alternate implementation"
#endif
  }
};

int
main() {
  return 0 != run_benchmark<std::variant, num_variants, seq_length, repeat_num, visitor_applier,
                            custom_clock>("std::variant", rng_seed);
}
