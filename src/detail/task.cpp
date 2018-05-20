#include <uthread/detail/task.hpp>

#include <cassert>

namespace uthread {
namespace detail {

namespace {

thread_local Context returnTo;
thread_local std::unique_ptr<Task> currentTask = nullptr;

}  // namespace

void Task::swapToTask(std::unique_ptr<Task> task, TaskQueue& queue) {
  assert(task);
  assert(currentTask);

  auto prevTask = currentTask.get();
  queue.push(std::move(currentTask));
  currentTask = std::move(task);
  contextSwap(prevTask->context_, currentTask->context_);
}

void Task::jumpToTask(std::unique_ptr<Task> task) {
  assert(task);
  assert(!currentTask);

  currentTask = std::move(task);
  contextSwap(returnTo, currentTask->context_);
}

void Task::runTask() {
  currentTask->f_->call();
  currentTask.reset();
  contextJump(returnTo);
}

void TaskQueue::push(std::unique_ptr<Task> task) {
  assert(task);

  if (!head_) {
    head_ = std::move(task);
    tail_ = head_.get();
  } else {
    tail_->next_ = std::move(task);
    tail_ = tail_->next_.get();
  }
}

std::unique_ptr<Task> TaskQueue::pop() {
  auto task = std::move(head_);
  if (task) {
    head_ = std::move(task->next_);
    if (!head_) {
      tail_ = nullptr;
    }
  }
  return task;
}

}  // namespace detail
}  // namespace uthread
