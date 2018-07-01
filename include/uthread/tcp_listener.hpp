#pragma once

#include <uthread/detail/task_fd.hpp>
#include <uthread/socket_addr.hpp>
#include <uthread/tcp_stream.hpp>

namespace uthread {

// A TCP connection for use in tasks. The operations are designed to only block
// the current executing task and not the kernel thread running the task loop.
class TcpListener {
 public:
  struct Options {
    // SO_REUSEADDR: http://man7.org/linux/man-pages/man7/socket.7.html
    bool reuseAddr = false;

    // backlog: http://man7.org/linux/man-pages/man2/listen.2.html
    int backlog = 16;
  };

  TcpListener() = default;

  // Creates a TCP listener bound to the specified address.
  TcpListener(SocketAddrV4 bindAddr, Options options);

  // Sleep the current task until a connection is established.
  //
  // Return an established connection.
  TcpStream accept();

 private:
  detail::TaskFd fd_;
};

}  // namespace uthread
