#pragma once

namespace uthread {
namespace detail {

// A task friendly file descriptor. Not only is this an RAII-based owner of the
// file, but operations on the file are made non-blocking so there is no threat
// of blocking the underlying kernel thread a task is running on.
struct TaskFd {
  TaskFd() = default;
  TaskFd(int fd);

  TaskFd(const TaskFd&) = delete;
  TaskFd(TaskFd&& fd);
  TaskFd& operator=(const TaskFd&) = delete;
  TaskFd& operator=(TaskFd&& fd);

  ~TaskFd();

  operator bool() const;

  // The file descriptor managed by this instance. Do NOT close this manually!
  int fd = -1;
};

}  // namespace detail
}  // namespace uthread
