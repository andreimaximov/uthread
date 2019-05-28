#include <gtest/gtest.h>

#include <uthread/detail/stack.hpp>

namespace uthread {
namespace detail {

TEST(StackTest, AccessStackOverflowGuardPage) {
  auto s1 = makeStack(1'024, true);
  auto s2 = makeStack(1'024, true);
  auto s3 = makeStack(1'024, true);
  (void)s2;
  (void)s3;

  auto p = s1.get();
  ASSERT_DEATH(*(p + getSysPageSize()) = '!', "");
}

}  // namespace detail
}  // namespace uthread
