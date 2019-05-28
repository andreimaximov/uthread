#pragma once

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

}  // namespace detail
}  // namespace uthread
