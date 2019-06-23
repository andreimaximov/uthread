#pragma once

#include <array>
#include <cstddef>
#include <memory>

namespace uthread {
namespace detail {

// Return the size of virtual memory pages used by the operating system.
std::size_t getSysPageSize();

// Creates a stack of the specified size. Will allocate an additional page
// to trigger a SIGSEGV (on *nix systems) in case of a stack overflow if
// useGuardPages is true.
std::shared_ptr<char> makeStack(std::size_t stackSize, bool useGuardPages);

template <std::size_t N = 65'536>
void tryToOverflowStack() {
  // Use volatile to trick compiler away from optimizing side-effect-less code.
  // This should reliably cause a SIGSEGV in ASAN or guard pages mode.
  std::array<char, N> xs;
  volatile char* p = &xs[0];
  while (p != &xs[0] + xs.size()) {
    *p = '!';
    p++;
  }
}

}  // namespace detail
}  // namespace uthread
