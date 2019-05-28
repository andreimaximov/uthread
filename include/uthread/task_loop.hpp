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

  TaskLoop(const TaskLoop&) = delete;
  TaskLoop(TaskLoop&&) = delete;
  TaskLoop& operator=(const TaskLoop&) = delete;
  TaskLoop& operator=(TaskLoop&&) = delete;

  // Schedule a task to run. This can be called both from outside or inside of
  // the loop.
  template <typename F>
  void addTask(F&& f) {
    readyTasks_.push(detail::Task::make(
        [this, f{std::forward<F>(f)}]() mutable {
          f();
          outstandingTasks_--;
        },
        options_.stackSize, options_.useGuardPages));
    outstandingTasks_++;
  }

  // Run all scheduled tasks and return once complete.
  void runLoop();

 private:
  // Suspend the current executing task by saving it on the queue. The loop will
  // switch to the next available task.
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
