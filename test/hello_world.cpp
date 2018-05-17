#include <memory>

#include <gtest/gtest.h>

#include <cpp/hello_world.hpp>

namespace cpp {

TEST(HelloWorldTest, Return42) {
  ASSERT_EQ(hello_world("Test"), 42);
}

TEST(HelloWorldTest, Asan) {
  std::shared_ptr<int> sp{new int{0}};
  auto p = sp.get();
  p += 10000;
  ASSERT_DEATH(*p = 42, "AddressSanitizer: heap-buffer-overflow on address");
}

}  // namespace cpp
