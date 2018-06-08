#include <uthread/detail/task.hpp>

#include <cassert>

namespace uthread {
namespace detail {

namespace {

thread_local Context returnTo;
thread_local std::unique_ptr<Task> currentTask = nullptr;

struct Trampoline {
  TaskQueue sleepQueue;
  TaskQueue& wakeQueue;
  int events;
};

static void callbackLibEvent(evutil_socket_t, short events, void* trampoline_) {
  auto trampoline = reinterpret_cast<Trampoline*>(trampoline_);
  trampoline->events = events;
  auto task = trampoline->sleepQueue.pop();
  assert(task);
  trampoline->wakeQueue.push(std::move(task));
}

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

short Task::sleepAndSwapToTask(int fd, short events, timeval* timeout,
                               event_base* evb, std::unique_ptr<Task> task,
                               TaskQueue& queue) {
  assert(evb);
  assert(task);
  assert(currentTask);

  Trampoline trampoline{{}, queue, 0};

  auto ev = reinterpret_cast<event*>(currentTask->event_.get());
  event_assign(ev, evb, fd, events, callbackLibEvent, &trampoline);
  event_add(ev, timeout);

  swapToTask(std::move(task), trampoline.sleepQueue);
  return trampoline.events;
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

bool TaskQueue::hasTasks() const {
  return !!head_;
}

}  // namespace detail
}  // namespace uthread
