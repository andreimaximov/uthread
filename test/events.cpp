#include <gtest/gtest.h>

#include <uthread/events.hpp>

namespace uthread {

TEST(EventsTest, OrAnd) {
  ASSERT_EQ((Events::Read | Events::Write) & Events::Read, Events::Read);
  ASSERT_EQ((Events::Read | Events::Write) & Events::Write, Events::Write);
}

}  // namespace uthread
