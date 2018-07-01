#pragma once

#include <cstdint>
#include <cstring>
#include <limits>
#include <string>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <uthread/exception.hpp>

namespace uthread {

struct SocketAddrV4 {
  // Creates an (network address, port) socket address.
  template <typename T>
  SocketAddrV4(const std::string& addr, T port) {
    if (port < 0 || port > std::numeric_limits<std::uint16_t>::max()) {
      throw Exception{"SocketAddrV4: Bad port."};
    }

    std::memset((char*)&socketAddr, 0, sizeof(socketAddr));
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_port = htons(static_cast<std::uint16_t>(port));

    if (addr.empty()) {
      socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else if (!inet_pton(AF_INET, addr.c_str(), &socketAddr.sin_addr.s_addr)) {
      throw Exception{"SocketAddrV4: Bad IPv4 address."};
    }
  }

  sockaddr_in socketAddr;
};

}  // namespace uthread
