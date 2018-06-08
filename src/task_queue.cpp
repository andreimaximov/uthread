#include <uthread/task_queue.hpp>

#include <uthread/task_loop.hpp>

namespace uthread {

void TaskQueue::park() {
  TaskLoop::current()->suspendTask(queue_);
}

bool TaskQueue::unpark() {
  auto task = queue_.pop();
  if (!task) {
    return false;
  }
  TaskLoop::current()->resumeTask(std::move(task));
  return true;
}

}  // namespace uthread
