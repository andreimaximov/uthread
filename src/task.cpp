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

  TaskLoop::current()->suspendTask(*joinQueue);
}

void Task::yield() {
  auto taskLoop = TaskLoop::current();
  if (UTHREAD_LIKELY(!!taskLoop)) {
    taskLoop->yieldTask();
  }
}

Events Task::sleep(int fd, Events events) {
  auto taskLoop = TaskLoop::current();
  auto fired = static_cast<Events>(detail::Task::sleepAndSwapToTask(
      fd, static_cast<short>(events), nullptr, taskLoop->evb_.get(),
      taskLoop->getNextTask(), taskLoop->readyTasks_));
  return fired;
}

}  // namespace uthread
