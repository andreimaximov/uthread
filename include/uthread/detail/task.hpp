#pragma once

#include <cstddef>
#include <memory>

#include <event2/event.h>
#include <sys/time.h>

#include <uthread/detail/context.hpp>
#include <uthread/detail/function.hpp>
#include <uthread/detail/stack.hpp>

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
  static std::unique_ptr<Task> make(F&& f, std::size_t stackSize = 16'384,
                                    bool useGuardPages = false) {
    std::unique_ptr<Task> task{new Task{}};
    task->f_ = makef(std::forward<F>(f));
    task->stack_ = makeStack(stackSize, useGuardPages);

    // Align event storage to 8 bytes since that's what malloc does on 64-bit
    // systems.
    task->event_ = std::unique_ptr<double[]>{
        new double[(event_get_struct_event_size() + 1) / sizeof(double)]};

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

  // Swap to another task while the current task waits for an event on the file
  // descriptor. The task is placed on the queue once an event fires.
  //
  // Return the fired events.
  static short sleepAndSwapToTask(int fd, short events, timeval* timeout,
                                  event_base* evb, std::unique_ptr<Task> task,
                                  TaskQueue& queue);

  template <typename F>
  static typename std::enable_if<FunctionReturnsVoid<F>>::type runInMainContext(
      F&& f) {
    FunctionT<F> fBase{std::forward<F>(f)};
    runInMainContextVoid(fBase);
  }

  template <typename F>
  static
      typename std::enable_if<FunctionReturnsRef<F>, FunctionReturnT<F>>::type
      runInMainContext(F&& f) {
    return *(runInMainContext([&f]() { return &(f()); }));
  }

  template <typename F>
  static
      typename std::enable_if<!FunctionReturnsVoid<F> && !FunctionReturnsRef<F>,
                              FunctionReturnT<F>>::type
      runInMainContext(F&& f) {
    static_assert(std::is_default_constructible<FunctionReturnT<F>>::value,
                  "Functions passed to runInMainContext(...) should return a "
                  "default constructible type.");
    FunctionReturnT<F> fReturn;
    auto fCaptureReturn = [&f, &fReturn]() { fReturn = f(); };
    FunctionT<decltype(fCaptureReturn)> fBase{fCaptureReturn};
    runInMainContextVoid(fBase);
    return fReturn;
  }

 private:
  Task() = default;

  static void runTask();

  static void runInMainContextVoid(FunctionBase& f);

  std::unique_ptr<FunctionBase> f_;
  std::unique_ptr<Task> next_;
  std::shared_ptr<char> stack_;
  std::unique_ptr<double[]> event_;
  Context context_;

  friend class TaskQueue;
};

class TaskQueue {
 public:
  void push(std::unique_ptr<Task> task);

  std::unique_ptr<Task> pop();

  bool hasTasks() const;

 private:
  std::unique_ptr<Task> head_;
  Task* tail_ = nullptr;
};

}  // namespace detail
}  // namespace uthread
