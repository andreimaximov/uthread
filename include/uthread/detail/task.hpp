#pragma once

#include <cstddef>
#include <memory>

#include <uthread/detail/context.hpp>
#include <uthread/detail/function.hpp>

namespace uthread {
namespace detail {

class TaskQueue;

// A snapshot of an execution context that can be paused and resumed.
class Task {
 public:
  Task(const Task&) = delete;
  Task(Task&&) = delete;
  Task& operator=(const Task&) = delete;
  Task& operator=(Task&&) = delete;

  // Makes a task which executes f when passed to swapToTask(...). When f
  // finishes executing the context switches to the last main context which
  // called jumpToTask(...).
  template <typename F>
  static std::unique_ptr<Task> make(F&& f, std::size_t stackSize = 16384) {
    std::unique_ptr<Task> task{new Task{}};
    task->f_ = makef(std::forward<F>(f));
    task->stack_ = std::unique_ptr<char[]>{new char[stackSize]};
    contextMake(task->context_, task->stack_.get(), stackSize, &Task::runTask);
    return task;
  }

  // Saves the execution context of the current task and switches to another
  // task. The currently executing task is pushed to the queue.
  static void swapToTask(std::unique_ptr<Task> task, TaskQueue& queue);

  // Saves the current execution context and switches to another task. This
  // should only be called from a "main" context. Hence there is no queue to
  // save the current task.
  static void jumpToTask(std::unique_ptr<Task> task);

 private:
  Task() = default;

  static void runTask();

  std::unique_ptr<FunctionBase> f_;
  std::unique_ptr<Task> next_;
  std::unique_ptr<char[]> stack_;
  Context context_;

  friend class TaskQueue;
};

class TaskQueue {
 public:
  void push(std::unique_ptr<Task> task);

  std::unique_ptr<Task> pop();

 private:
  std::unique_ptr<Task> head_;
  Task* tail_ = nullptr;
};

}  // namespace detail
}  // namespace uthread
