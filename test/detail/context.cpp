#include <gtest/gtest.h>

#include <uthread/detail/context.hpp>

namespace uthread {
namespace detail {

// clang-format off
// ----------------------------------------------------------------------------
// WARNING: The context API is not ASAN friendly but can be made so via some
// hooks the sanitizer provides:
//
// - https://github.com/google/sanitizers/issues/189
// - https://github.com/llvm-mirror/compiler-rt/blob/master/include/sanitizer/common_interface_defs.h#L176-L196
// - https://github.com/llvm-mirror/compiler-rt/blob/master/test/asan/TestCases/Linux/swapcontext_annotation.cc
// - https://github.com/boostorg/context/issues/65
// ----------------------------------------------------------------------------
// clang-format on

namespace {

constexpr std::size_t kStackSize = 1024 * 16;
int x = 0;
int y = 0;
char stackf[kStackSize];
char stackg[kStackSize];
char stackh[kStackSize];
Context contextMain{};
Context contextf{};
Context contextg{};
Context contexth{};

void f() {
  ASSERT_EQ(x, 0);
  x = 1;
  contextJump(contextMain);
}

void g() {
  ASSERT_EQ(y, 0);
  contextSwap(contextg, contexth);
  ASSERT_EQ(y, 1);
  contextSwap(contextg, contexth);
  ASSERT_EQ(y, 2);
  contextJump(contextMain);
}

void h() {
  ASSERT_EQ(y, 0);
  y = 1;
  contextSwap(contexth, contextg);
  ASSERT_EQ(y, 1);
  y = 2;
  contextSwap(contexth, contextg);
}

}  // namespace

TEST(ContextTest, SwapToChildAndJumpBack) {
  contextMake(contextf, stackf, sizeof(stackf), f);
  contextSwap(contextMain, contextf);

  ASSERT_EQ(x, 1);
}

TEST(ContextTest, SwapBetweenChildrenAndJumpBack) {
  contextMake(contextg, stackg, sizeof(stackg), g);
  contextMake(contexth, stackh, sizeof(stackh), h);
  contextSwap(contextMain, contextg);

  ASSERT_EQ(y, 2);
}

}  // namespace detail
}  // namespace uthread
