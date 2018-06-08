#pragma once

#include <memory>

#include <event2/event.h>

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
    readyTasks_.push(detail::Task::make(
        [f{std::forward<F>(f)}]() {
          f();
          TaskLoop::current()->outstandingTasks_--;
        },
        options_.stackSize));
    outstandingTasks_++;
  }

  // Run all scheduled tasks and return once complete.
  void runLoop();

 private:
  // Suspend the current executing task by saving it on the queue. The loop will
  // switch to the next available task. If no tasks are ready to run, an
  // exception is thrown.
  void suspendTask(detail::TaskQueue& queue);

  // Schedule the task to resume execution.
  void resumeTask(std::unique_ptr<detail::Task> task);

  // Swap out the current executing task if another ready tasks exists.
  void yieldTask();

  // Return the next schedule task.
  std::unique_ptr<detail::Task> getNextTask();

  // Return the current executing loop. Throw an exception if no loop is
  // currently executing.
  static TaskLoop* current();

  detail::TaskQueue readyTasks_;
  const Options options_;
  std::size_t outstandingTasks_ = 0;
  std::shared_ptr<event_base> evb_;

  friend class Task;
  friend class TaskQueue;
};

}  // namespace uthread
