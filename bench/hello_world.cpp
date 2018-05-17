#include <benchmark/benchmark.h>

#include <cpp/hello_world.hpp>

namespace cpp {

static void bench_hello_world(benchmark::State& state) {
  for (auto _ : state) {
    hello_world("Benchmark");
  }
}

BENCHMARK(bench_hello_world);

}  // namespace cpp
