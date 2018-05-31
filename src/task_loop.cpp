#include <uthread/task_loop.hpp>

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

TaskLoop::TaskLoop(Options options) : options_{options} {}

void TaskLoop::runLoop() {
  TaskLoop*& taskLoop = TaskLoop::current();
  if (taskLoop) {
    throw Exception{"Task loop is already running."};
  }

  taskLoop = this;
  runLoopNoThrow(readyTasks_);
  taskLoop = nullptr;
}

void TaskLoop::suspendTask(detail::TaskQueue& queue) {
  auto goTo = currentSafe().readyTasks_.pop();
  if (UTHREAD_UNLIKELY(!goTo)) {
    throw Exception{"Suspending task causes deadlock."};
  }

  detail::Task::swapToTask(std::move(goTo), queue);
}

void TaskLoop::resumeTask(std::unique_ptr<detail::Task> task) {
  currentSafe().readyTasks_.push(std::move(task));
}

TaskLoop*& TaskLoop::current() {
  static TaskLoop* taskLoop = nullptr;
  return taskLoop;
}

TaskLoop& TaskLoop::currentSafe() {
  auto taskLoop = current();
  if (UTHREAD_UNLIKELY(!taskLoop)) {
    throw Exception{"Task loop not running."};
  }
  return *taskLoop;
}

}  // namespace uthread
