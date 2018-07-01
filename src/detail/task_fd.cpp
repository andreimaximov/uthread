#include <uthread/detail/task_fd.hpp>

#include <fcntl.h>
#include <unistd.h>

#include <uthread/exception.hpp>

namespace uthread {
namespace detail {

TaskFd::TaskFd(int fd) : fd{fd} {
  if (fd < 0) {
    throw Exception{"TaskFd: Cannot make from descriptor < 0."};
  }

  auto flags = fcntl(fd, F_GETFL);
  if (flags == -1) {
    throw Exception::fromErrNo();
  }

  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    throw Exception::fromErrNo();
  }
}

TaskFd::TaskFd(TaskFd&& fd) {
  *this = std::move(fd);
}

TaskFd& TaskFd::operator=(TaskFd&& fd) {
  if (this->fd >= 0) {
    close(this->fd);
  }
  this->fd = fd.fd;
  fd.fd = -1;
  return *this;
}

TaskFd::~TaskFd() {
  if (fd >= 0) {
    close(fd);
  }
}

TaskFd::operator bool() const {
  return fd >= 0;
}

}  // namespace detail
}  // namespace uthread
