#include <benchmark/benchmark.h>

#include <uthread/detail/context.hpp>

namespace uthread {
namespace detail {

// This benchmarks estimates the cost of a context snapshot and switch using
// the uthread context API.
static void benchContextUThread(benchmark::State& state) {
  for (auto _ : state) {
    Context context;
    contextSwap(context, context);
  }
}

BENCHMARK(benchContextUThread);

#ifdef __linux__

#include <ucontext.h>

// This benchmarks estimates the cost of a context snapshot and switch using
// the Linux context API.
static void benchContextLinux(benchmark::State& state) {
  for (auto _ : state) {
    volatile bool skip = false;
    ucontext_t context;
    getcontext(&context);
    if (!skip) {
      skip = true;
      setcontext(&context);
    }
  }
}

BENCHMARK(benchContextLinux);

#endif

}  // namespace detail
}  // namespace uthread
