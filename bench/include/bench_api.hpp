#pragma once

// Extracted from google benchmarks suite
// c.f. https://github.com/google/benchmark/blob/master/include/benchmark/benchmark_api.h
//
// These two functions DoNotOptimize and ClobberMemory prevent the compiler
// from optimizing in certain ways.

namespace benchmark {

#if defined(__GNUC__)
#define BENCHMARK_ALWAYS_INLINE __attribute__((always_inline))
#else
#define BENCHMARK_ALWAYS_INLINE
#endif


#if defined(__GNUC__)
template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
    asm volatile("" : : "g"(value) : "memory");
}
// Force the compiler to flush pending writes to global memory. Acts as an
// effective read/write barrier
inline BENCHMARK_ALWAYS_INLINE void ClobberMemory() {
    asm volatile("" : : : "memory");
}

#else

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
    static_cast<volatile void>(&reinterpret_cast<char const volatile&>(value)));
}

// TODO
template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void ClobberMemory() {}

#endif

} // end namespace benchmark
