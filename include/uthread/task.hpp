#pragma once

#include <memory>

#include <uthread/detail/task.hpp>
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
    auto joinQueue = std::make_shared<detail::TaskQueue>();

    TaskLoop::currentSafe().addTask([f{std::move(f)}, joinQueue]() {
      auto fNoThrow = [&f]() noexcept { f(); };
      fNoThrow();

      // Resume all joined tasks.
      while (auto task = joinQueue->pop()) {
        TaskLoop::resumeTask(std::move(task));
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

 private:
  std::weak_ptr<detail::TaskQueue> joinQueue_;
};

}  // namespace uthread
