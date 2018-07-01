#include <gtest/gtest.h>

#include <uthread/exception.hpp>
#include <uthread/socket_addr.hpp>

namespace uthread {

TEST(SocketAddrTest, FromOkStr) {
  SocketAddrV4 addr{"128.0.0.1", 8080};
}

TEST(SocketAddrTest, FromBadStr) {
  ASSERT_THROW(SocketAddrV4 addr("Wha!", 8080), Exception);
}

}  // namespace uthread
