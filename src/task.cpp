#include <uthread/task.hpp>

#include <cassert>

#include <uthread/task_loop.hpp>

namespace uthread {

Task::Task(Task&& task) {
  *this = std::move(task);
}

Task& Task::operator=(Task&& task) {
  joinQueue_ = std::move(task.joinQueue_);
  task.joinQueue_.reset();
  return *this;
}

void Task::join() {
  assert(TaskLoop::current());

  auto joinQueue = joinQueue_.lock();
  if (!joinQueue) {
    // Task has finished.
    return;
  }

  TaskLoop::suspendTask(*joinQueue);
}

void Task::yield() {
  auto taskLoop = TaskLoop::current();
  if (!taskLoop) {
    return;
  }

  auto goTo = taskLoop->readyTasks_.pop();
  if (goTo) {
    detail::Task::swapToTask(std::move(goTo), taskLoop->readyTasks_);
  }
}

}  // namespace uthread