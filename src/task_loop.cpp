#include <uthread/task_loop.hpp>

#include <cassert>

#include <uthread/detail/likely.hpp>
#include <uthread/exception.hpp>

namespace uthread {

namespace {

void runLoopNoThrow(detail::TaskQueue& tasks) noexcept {
  while (auto task = tasks.pop()) {
    detail::Task::jumpToTask(std::move(task));
  }
}

}  // namespace

void TaskLoop::runLoop() {
  if (TaskLoop::current()) {
    throw Exception{"Task loop is already running."};
  }

  TaskLoop::current() = this;
  runLoopNoThrow(readyTasks_);
  TaskLoop::current() = nullptr;
}

void TaskLoop::suspendTask(detail::TaskQueue& queue) {
  auto taskLoop = TaskLoop::current();
  assert(taskLoop);

  auto goTo = taskLoop->readyTasks_.pop();
  if (UTHREAD_UNLIKELY(!goTo)) {
    throw Exception{"Suspending task causes deadlock."};
  }

  detail::Task::swapToTask(std::move(goTo), queue);
}

TaskLoop*& TaskLoop::current() {
  static TaskLoop* taskLoop = nullptr;
  return taskLoop;
}

}  // namespace uthread
