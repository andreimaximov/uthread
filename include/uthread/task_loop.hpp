#pragma once

#include <uthread/detail/task.hpp>
#include <uthread/options.hpp>

namespace uthread {

// A loop for scheduling tasks to run.
class TaskLoop {
 public:
  TaskLoop(Options options = Options{});

  // Schedule a task to run. This can be called both from outside or inside of
  // the loop.
  template <typename F>
  void addTask(F&& f) {
    readyTasks_.push(
        detail::Task::make(std::forward<F>(f), options_.stackSize));
  }

  // Run all scheduled tasks and return once complete.
  void runLoop();

 private:
  // Suspend the current executing task by saving it on the queue. The loop will
  // switch to the next available task. If no tasks are ready to run, an
  // exception is thrown.
  static void suspendTask(detail::TaskQueue& queue);

  // Schedule the task to resume execution.
  static void resumeTask(std::unique_ptr<detail::Task> task);

  static TaskLoop*& current();

  static TaskLoop& currentSafe();

  detail::TaskQueue readyTasks_;
  const Options options_;

  friend class Task;
  friend class TaskQueue;
};

}  // namespace uthread
