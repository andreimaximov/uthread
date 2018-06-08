#include <benchmark/benchmark.h>

#include <uthread/task.hpp>
#include <uthread/task_loop.hpp>

namespace uthread {

// This benchmarks the estimated cost of yield from one task to another and
// back. The task loop actually spawns one more task for the event base loop
// which we end up switching to during this cycle so this benchmarks not 2
// yields, but 3.
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
