#include <uthread/tcp_listener.hpp>

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <uthread/events.hpp>
#include <uthread/exception.hpp>
#include <uthread/task.hpp>

namespace uthread {

TcpListener::TcpListener(SocketAddrV4 bindAddr, Options options)
    : fd_{socket(AF_INET, SOCK_STREAM, 0)} {
  int opt = 1;
  if ((options.reuseAddr && setsockopt(fd_.fd, SOL_SOCKET, SO_REUSEADDR,
                                       (const void*)&opt, sizeof(opt)) == -1) ||
      (bind(fd_.fd, (sockaddr*)&bindAddr.socketAddr,
            sizeof(bindAddr.socketAddr)) == -1) ||
      (listen(fd_.fd, options.backlog) == -1)) {
    throw Exception::fromErrNo();
  }
}

TcpStream TcpListener::accept() {
  while (true) {
    Task::sleep(fd_.fd, Events::Read);
    auto conn = ::accept(fd_.fd, nullptr, nullptr);
    if (conn == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
      throw Exception::fromErrNo();
    } else if (conn != -1) {
      TcpStream stream;
      stream.fd_ = detail::TaskFd{conn};
      return stream;
    }
  }
}

}  // namespace uthread
