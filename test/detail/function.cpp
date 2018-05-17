#include <exception>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <uthread/detail/function.hpp>

namespace uthread {
namespace detail {

namespace {

struct NonCopyable {
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable(NonCopyable&&) = default;
  NonCopyable& operator=(const NonCopyable&) = delete;
  NonCopyable& operator=(NonCopyable&&) = default;
};

}  // namespace

using testing::MockFunction;
using testing::StrictMock;
using testing::Throw;

TEST(FunctionTest, MakeAndCall) {
  StrictMock<MockFunction<void()>> f1;
  EXPECT_CALL(f1, Call()).Times(2);

  auto f2 = makef(f1.AsStdFunction());
  f2->call();
  f2->call();
}

TEST(FunctionTest, MakeNonCopyableLambda) {
  NonCopyable x;
  auto f = makef([x{std::move(x)}]() {});
  f->call();
}

TEST(FunctionTest, Throw) {
  ASSERT_DEATH(
      []() {
        StrictMock<MockFunction<void()>> f1;
        EXPECT_CALL(f1, Call()).WillOnce(Throw(std::exception{}));

        auto f2 = makef(f1.AsStdFunction());
        f2->call();
      }(),
      "");
}

}  // namespace detail
}  // namespace uthread
