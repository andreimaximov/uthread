#pragma once

#include <chrono>
#include <memory>

#include <sys/time.h>

#include <uthread/detail/task.hpp>
#include <uthread/events.hpp>
#include <uthread/task_loop.hpp>

namespace uthread {

// A stack contains its own stack frame used to execute a provided function f.
class Task {
 public:
  // Creates a task which does not execute anything.
  Task() = default;

  // Creates a task which will execute function f when scheduled by the loop.
  // You can only create tasks from within the task loop.
  template <typename F>
  Task(F&& f) {
    auto taskLoop = TaskLoop::current();
    auto joinQueue = std::make_shared<detail::TaskQueue>();

    taskLoop->addTask([f{std::move(f)}, taskLoop, joinQueue]() mutable {
      auto fNoThrow = [&f]() noexcept { f(); };
      fNoThrow();

      // Resume all joined tasks.
      while (auto task = joinQueue->pop()) {
        taskLoop->resumeTask(std::move(task));
      }
    });

    joinQueue_ = joinQueue;
  }

  // Tasks are movable but not copyable. There are 2 cases to consider when
  // moving task into this:
  //
  // 1. this refers to an executing task
  // 2. this does NOT refer to an executing task
  //
  // In either case the state of the task is transfered to this. In the 1st
  // case, the previous state of this continues executing but the handle is
  // lost.
  Task(const Task&) = delete;
  Task(Task&& task);
  Task& operator=(const Task&) = delete;
  Task& operator=(Task&& task);

  // Sleeps the current executing task until this task is finished. If the task
  // is finished or was not executable in the first place, return immediately.
  void join();

  // Yield execution of the current task to another. Return immediately if there
  // are no other runnable tasks.
  static void yield();

  // Sleep the current task until an event fires on the file descriptor.
  //
  // Return the fired events.
  static Events sleep(int fd, Events events);

  // Sleep the current task for the specified duration.
  template <typename R, typename P>
  static void sleep(std::chrono::duration<R, P> duration) {
    auto taskLoop = TaskLoop::current();

    auto durationS = std::chrono::duration_cast<std::chrono::seconds>(duration);
    auto durationUs = std::chrono::duration_cast<std::chrono::microseconds>(
        duration - durationS);

    timeval durationTv;
    durationTv.tv_sec = durationS.count();
    durationTv.tv_usec = durationUs.count();

    detail::Task::sleepAndSwapToTask(
        -1, EV_TIMEOUT, &durationTv, taskLoop->evb_.get(),
        taskLoop->getNextTask(), taskLoop->readyTasks_);
  }

  // Invoke a function f from the main context. This can be called from outside
  // a task context in which case f will be invoked inline. As of now the return
  // types of f are limited to default constructible types. This means returning
  // references will not work - use a pointer instead.
  template <typename F>
  static decltype(auto) runInMainContext(F&& f) {
    return detail::Task::runInMainContext(std::forward<F>(f));
  }

 private:
  std::weak_ptr<detail::TaskQueue> joinQueue_;
};

}  // namespace uthread
