#pragma once

#include <uthread/detail/task_fd.hpp>
#include <uthread/socket_addr.hpp>

namespace uthread {

// A TCP connection for use in tasks. The operations are designed to only block
// the current executing task and not the kernel thread running the task loop.
class TcpStream {
 public:
  TcpStream() = default;

  // Creates a TCP stream connected to the specified address.
  TcpStream(SocketAddrV4 connectAddr);

  // Send bufLen bytes from buf via the stream.
  //
  // Return the number of bytes sent from buf. A return of 0 for a non-zero
  // bufLen means the socket on the other end has closed.
  std::size_t send(const void* buf, std::size_t bufLen);

  // Read up to bufLen bytes into buf.
  //
  // Return the number of bytes read into buf. A return of 0 for a non-zero
  // bufLen means the socket on the other end has closed.
  std::size_t read(void* buf, std::size_t bufLen);

 private:
  detail::TaskFd fd_;

  friend class TcpListener;
};

}  // namespace uthread
