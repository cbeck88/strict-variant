#include "bench_framework.hpp"
#include <safe_variant/variant.hpp>

static constexpr uint32_t num_variants{NUM_VARIANTS};
static constexpr uint32_t seq_length{SEQ_LENGTH};
static constexpr uint32_t repeat_num{REPEAT_NUM};
static constexpr uint32_t rng_seed{RNG_SEED};

struct visitor_applier {
  template <typename T>
  uint32_t operator()(T && t) const {
    return safe_variant::apply_visitor(benchmark::visitor{}, std::forward<T>(t));
  }
};

int
main() {
  benchmark::run_benchmark<safe_variant::variant, num_variants, seq_length, repeat_num,
                           visitor_applier>("safe_variant::variant", rng_seed);
}
