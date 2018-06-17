#pragma once

#include <cstddef>
#include <queue>

#include <uthread/task_queue.hpp>

namespace uthread {

// A blocking queue of T's for use by tasks running in the same loop.
template <typename T>
class MpmcQueue {
 public:
  // Creates a queue with unbounded capacity.
  MpmcQueue() : MpmcQueue{0} {}

  // Creates a queue with the specified capacity.
  //
  // A size of 0 indicates unbounded capacity.
  explicit MpmcQueue(std::size_t capacity) : capacity_{capacity} {}

  // Pop an element from the head queue.
  //
  // The current task sleeps until an element becomes available.
  void pop(T& value) {
    if (queue_.size() == 0) {
      pop_.park();
    }

    try {
      value = std::move(queue_.front());
    } catch (...) {
      // We may have been unpark'd by a push. If an exception is thrown we
      // should unpark another sleeping pop'er.
      pop_.unpark();
      throw;
    }

    queue_.pop();
    push_.unpark();
  }

  // Push an element onto the tail queue.
  //
  // The current task sleeps if the queue is at full capacity.
  void push(T value) {
    if (capacity_ != 0 && queue_.size() == capacity_) {
      push_.park();
    }

    try {
      queue_.push(std::move(value));
    } catch (...) {
      // We may have been unpark'd by a pop. If an exception is thrown we should
      // unpark another sleeping push'er.
      push_.unpark();
      throw;
    }

    pop_.unpark();
  }

 private:
  std::size_t capacity_;
  std::queue<T> queue_;
  TaskQueue pop_;
  TaskQueue push_;
};

}  // namespace uthread
