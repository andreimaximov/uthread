#include <uthread/tcp_stream.hpp>

#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <uthread/events.hpp>
#include <uthread/exception.hpp>
#include <uthread/task.hpp>

namespace uthread {

TcpStream::TcpStream(SocketAddrV4 connectAddr)
    : fd_{socket(AF_INET, SOCK_STREAM, 0)} {
  // https://nwat.xyz/blog/2014/01/16/porting-msg_more-and-msg_nosigpipe-to-osx/
#ifndef MSG_NOSIGNAL
  int opt = 1;
  if (setsockopt(fd_.fd, SOL_SOCKET, SO_NOSIGPIPE, (const void*)&opt,
                 sizeof(opt)) == -1) {
    throw Exception::fromErrNo();
  }
#endif

  while (true) {
    if (connect(fd_.fd, (sockaddr*)&connectAddr.socketAddr,
                sizeof(connectAddr.socketAddr)) == 0 ||
        errno == EISCONN) {
      break;
    } else if (!(errno == EINPROGRESS || errno == EALREADY)) {
      throw Exception::fromErrNo();
    }
    Task::sleep(fd_.fd, Events::Write);
  }
}

std::size_t TcpStream::send(const void* buf, std::size_t bufLen) {
  if (!fd_) {
    throw Exception{"TcpStream: No connection."};
  }

  std::size_t p = 0;

  while (p < bufLen) {
    auto events = Task::sleep(fd_.fd, Events::Read | Events::Write);
    if ((events & Events::Write) == Events::Write) {
#ifndef MSG_NOSIGNAL
      int flags = 0;
#else
      // https://nwat.xyz/blog/2014/01/16/porting-msg_more-and-msg_nosigpipe-to-osx/
      int flags = MSG_NOSIGNAL;
#endif

      auto sendLen =
          ::send(fd_.fd, static_cast<const char*>(buf) + p, bufLen - p, flags);
      if (sendLen == -1 && errno == EPIPE) {
        // Connection has closed.
        return 0;
      } else if (sendLen == -1) {
        throw Exception::fromErrNo();
      }

      p += sendLen;
    } else if ((events & Events::Read) == Events::Read &&
               recv(fd_.fd, &p, sizeof(p), MSG_PEEK | MSG_DONTWAIT) == 0) {
      // Connection has closed:
      // http://stefan.buettcher.org/cs/conn_closed.html
      return 0;
    }
  }

  return p;
}

std::size_t TcpStream::read(void* buf, std::size_t bufLen) {
  if (!fd_) {
    throw Exception{"TcpStream: No connection."};
  }

  Task::sleep(fd_.fd, Events::Read);
  auto readLen = recv(fd_.fd, buf, bufLen, 0);
  if (readLen < 0) {
    throw Exception::fromErrNo();
  }
  return readLen;
}

}  // namespace uthread
