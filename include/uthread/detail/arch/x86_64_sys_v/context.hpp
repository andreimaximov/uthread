#pragma once

#include <cstdint>

namespace uthread {
namespace detail {

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
