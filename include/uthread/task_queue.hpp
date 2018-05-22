#pragma once

#include <uthread/detail/task.hpp>

namespace uthread {

// A FIFO queue for sleeping and resuming execution of tasks.
class TaskQueue {
 public:
  // Sleeps the current executing task on the tail of the queue until it is
  // unparked.
  void park();

  // Awaken the task at the head of the queue.
  //
  // Return true if a task was awakened and false otherwise.
  bool unpark();

 private:
  detail::TaskQueue queue_;
};

}  // namespace uthread
