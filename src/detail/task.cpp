#include <uthread/detail/task.hpp>

#include <cassert>

namespace uthread {
namespace detail {

namespace {

thread_local Context returnTo;
thread_local std::unique_ptr<Task> currentTask = nullptr;
thread_local std::unique_ptr<Task> cleanupTask = nullptr;
thread_local FunctionBase* fRunInMain = nullptr;

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

  if (cleanupTask) {
    cleanupTask.reset();
  }
}

void Task::jumpToTask(std::unique_ptr<Task> task) {
  assert(task);
  assert(!currentTask);

  currentTask = std::move(task);

  while (currentTask || fRunInMain) {
    if (fRunInMain) {
      // We MUST have switched back from a task context.
      assert(currentTask);

      // Indicate we are running in the main context in case runInMainContext is
      // called from fRunInMain.
      auto currentTaskTmp = std::move(currentTask);
      fRunInMain->call();
      fRunInMain = nullptr;
      currentTask = std::move(currentTaskTmp);
    }

    if (currentTask) {
      contextSwap(returnTo, currentTask->context_);
    }
  }

  if (cleanupTask) {
    cleanupTask.reset();
  }
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

  // Careful here, do not destroy the stack yet while the context is still
  // technically running. This can cause SIGSEGV or worse - UB!
  cleanupTask = std::move(currentTask);
  contextJump(returnTo);
}

void Task::runInMainContextVoid(FunctionBase& f) {
  // Called from a main context so we can execute f inline.
  if (!currentTask) {
    f.call();
    return;
  }

  // There should be no other functions enqueued for execution. Stash a pointer
  // to the target function. This will be picked up in the main context and ran.
  assert(!fRunInMain);
  fRunInMain = &f;
  contextSwap(currentTask->context_, returnTo);
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
