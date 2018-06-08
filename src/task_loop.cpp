#include <uthread/task_loop.hpp>

#include <cassert>

#include <uthread/detail/likely.hpp>
#include <uthread/exception.hpp>
#include <uthread/task.hpp>

namespace uthread {

namespace {

static thread_local TaskLoop* taskLoop = nullptr;

void runLoopNoThrow(detail::TaskQueue& tasks) noexcept {
  while (auto task = tasks.pop()) {
    detail::Task::jumpToTask(std::move(task));
  }
}

std::shared_ptr<event_base> makeEvb(const Options& options) {
  std::shared_ptr<event_config> config;
  if (auto conf = event_config_new()) {
    config = std::shared_ptr<event_config>{conf, event_config_free};
  } else {
    throw Exception{"LibEvent: Error making config."};
  }

  if (event_config_set_flag(config.get(), EVENT_BASE_FLAG_NOLOCK)) {
    throw Exception{"LibEvent: Error disabling locking."};
  }

  if (options.timer == Options::Timer::Fast &&
      event_config_set_flag(config.get(), EVENT_BASE_FLAG_PRECISE_TIMER)) {
    throw Exception{"LibEvent: Error enabling precise timer."};
  }

  if (auto evb = event_base_new_with_config(config.get())) {
    return std::shared_ptr<event_base>{evb, event_base_free};
  } else {
    throw Exception{"LibEvent: Error making event base."};
  }
}

}  // namespace

TaskLoop::TaskLoop(Options options)
    : options_{options}, evb_{makeEvb(options_)} {}

void TaskLoop::runLoop() {
  if (taskLoop) {
    throw Exception{"Task loop is already running."};
  }

  addTask([this]() {
    // Are there any tasks which might still perform IO?
    while (outstandingTasks_ > 1) {
      // Are any tasks ready to run now? In such a case, don't block.
      auto flags = readyTasks_.hasTasks() ? EVLOOP_NONBLOCK : EVLOOP_ONCE;
      if (event_base_loop(evb_.get(), flags) == -1) {
        throw Exception{"Event base loop encountered an error."};
      }
      Task::yield();
    }
  });

  taskLoop = this;
  runLoopNoThrow(readyTasks_);
  taskLoop = nullptr;
}

void TaskLoop::suspendTask(detail::TaskQueue& queue) {
  detail::Task::swapToTask(getNextTask(), queue);
}

void TaskLoop::resumeTask(std::unique_ptr<detail::Task> task) {
  readyTasks_.push(std::move(task));
}

void TaskLoop::yieldTask() {
  // Yield from event base loop might not have another ready task.
  if (auto task = getNextTask()) {
    detail::Task::swapToTask(std::move(task), readyTasks_);
  }
}

std::unique_ptr<detail::Task> TaskLoop::getNextTask() {
  return readyTasks_.pop();
}

TaskLoop* TaskLoop::current() {
  if (UTHREAD_UNLIKELY(!taskLoop)) {
    throw Exception{"Task loop not running."};
  }
  return taskLoop;
}

}  // namespace uthread
