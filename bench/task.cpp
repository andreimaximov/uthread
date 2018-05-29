#include <benchmark/benchmark.h>

#include <uthread/task.hpp>
#include <uthread/task_loop.hpp>

namespace uthread {

// This benchmarks the estimated cost of yield from one task to another and
// back.
static void benchTaskYield(benchmark::State& state) {
  auto done = false;
  TaskLoop taskLoop;

  taskLoop.addTask([&done]() {
    while (!done) {
      Task::yield();
    }
  });

  taskLoop.addTask([&state, &done]() {
    for (auto _ : state) {
      Task::yield();
    }
    done = true;
  });

  taskLoop.runLoop();
}

BENCHMARK(benchTaskYield);

}  // namespace uthread
