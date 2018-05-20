#pragma once

#include <cstdint>

namespace uthread {
namespace detail {

// TODO(amaximov): Try alignas + over-aligned operator new (C++ 17) to align and
// fit the context in one 64 byte cache line.
struct Context {
  std::uint64_t rbx;
  std::uint64_t rbp;
  std::uint64_t rsp;
  std::uint64_t rip;
  std::uint64_t r12;
  std::uint64_t r13;
  std::uint64_t r14;
  std::uint64_t r15;
};

}  // namespace detail
}  // namespace uthread
