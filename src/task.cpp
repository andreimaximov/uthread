#include <uthread/task.hpp>

#include <uthread/detail/likely.hpp>
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
  if (UTHREAD_LIKELY(!!goTo)) {
    detail::Task::swapToTask(std::move(goTo), taskLoop->readyTasks_);
  }
}

}  // namespace uthread
