#include <benchmark/benchmark.h>

#include <uthread/detail/task.hpp>

namespace uthread {
namespace detail {

// This benchmarks the estimated cost of swapping from one task to another and
// back.
static void benchTaskSwap(benchmark::State& state) {
  TaskQueue queue;
  auto done = false;
  auto siblingTask = Task::make([&queue, &done]() {
    while (!done) {
      auto benchTask = queue.pop();
      Task::swapToTask(std::move(benchTask), queue);
    }
  });

  queue.push(std::move(siblingTask));

  auto benchTask = Task::make([&state, &queue, &done]() {
    for (auto _ : state) {
      auto siblingTask = queue.pop();
      Task::swapToTask(std::move(siblingTask), queue);
    }
    done = true;
  });

  Task::jumpToTask(std::move(benchTask));
  Task::jumpToTask(queue.pop());
}

BENCHMARK(benchTaskSwap);

}  // namespace detail
}  // namespace uthread
